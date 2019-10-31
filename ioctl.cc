#include <node_api.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/ioctl.h>

// NAPI METHOD: napi_ioctl
napi_value napi_ioctl (napi_env env, napi_callback_info info) {
  int rc = 0;
  char code[4];
  size_t argc = 3;
  napi_value argv[3];
  napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
  
  // Check if there are 3 arguments
  if (argc < 3) {
    napi_throw_error(
      env,
      NULL,
      "Wrong number of arguments. Expected 3");
    return NULL;
  }

  // Get fd (argument 1) value 
  int32_t fd;
  if (napi_get_value_int32(env, argv[0], &fd) != napi_ok) {
    napi_throw_error(env, "EINVAL", "Expected number");
    return NULL;
  }

  // Get request (argument 2) value 
  uint32_t request;
  if (napi_get_value_uint32(env, argv[1], &request) != napi_ok) { \
    napi_throw_error(env, "EINVAL", "Expected unsigned number"); \
    return NULL;
  }

  // Get argp (argument 3) value type (int or object)
  napi_valuetype valuetype2;
  napi_typeof(env, argv[2], &valuetype2);
  if (valuetype2 == napi_number) {
    // napi_number
    // Get argp (argument 3) value as integer
    int32_t argp;
    if (napi_get_value_int32(env, argv[2], &argp) != napi_ok) {
      napi_throw_error(env, "EINVAL", "Expected number");
      return NULL;
    }
    rc = ioctl(fd, request, argp);
  } else {
    // napi_object
    // Get argp (argument 3) value as object
    void * argp;
    size_t argp_len;
    if (napi_get_buffer_info(env, 
      argv[2], (void **) &argp, &argp_len) != napi_ok) {
      napi_throw_error(env, NULL, "napi_get_buffer_info() failed");
      return NULL;
    }
    rc = ioctl(fd, request, argp);
  }

  // Return error if ioctl result is less than 0
  if (rc < 0) {
    sprintf(code, "%d", errno);
    napi_throw_error(env,code,(const char *) strerror(errno));
  }

  // Retrun ioctl result
  napi_value return_int32;
  napi_create_int32(env, rc, &return_int32);
  return return_int32;
}

// NAPI METHOD: napi_memAddress
napi_value napi_memAddress (napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value argv[1];
  napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
  char *memAddress;
  
  // Check if there is 1 argument
  if (argc < 1) {
    napi_throw_error(
      env,
      NULL,
      "Wrong number of arguments. Expected 1");
    return NULL;
  }

  // Check if argument 1 is a Buffer
  bool is_buffer;
  napi_is_buffer(env, argv[0], &is_buffer);
  if (!is_buffer) {
    napi_throw_error(
      env,
      NULL,
      "Buffer instance expected");
    return NULL;
  }

  // Get Buffer
  char * argp;     // Argument pointer (Buffer is cast to char *)
  size_t argp_len; // Argument length  (Buffer length result)
  if (napi_get_buffer_info(env, argv[0], 
      (void **) &argp, &argp_len) != napi_ok) {
    napi_throw_error(env, NULL, "napi_get_buffer_info() failed");
    return NULL;
  }

  // Get Buffer address 
  // (Should print 16 digits if 64-bit arch and 8 digits if 32-bit arch)
  int ret = asprintf(&memAddress, "%0*" PRIxPTR, 
    (int)sizeof(uintptr_t)*2, (uintptr_t)argp);
  if (ret < 0) {
    napi_throw_error(env, NULL, 
      "asprintf() failed, error allocating memory for address string");
    return NULL;
  }

  // Return Buffer address as string
  napi_value return_utf8;
  napi_create_string_utf8(env, memAddress, strlen(memAddress), &return_utf8);
  return return_utf8;
}

// NAPI INIT
napi_value Init(napi_env env, napi_value exports) {
  napi_status status;
  napi_value napi_function;

  // Create ioctl function
  status = napi_create_function(
    env,
    "ioctl",
    NAPI_AUTO_LENGTH,
    napi_ioctl,
    NULL,
    &napi_function);
  if ( status != napi_ok) {
    napi_throw_error(env, NULL, "napi_create_function() failed!");
  }

  // Set ioctl function in exports
  status = napi_set_named_property(
    env,
    exports,
    "ioctl",
    napi_function);
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "napi_set_named_property() failed!");
  }

  // Create memAddress function
  status = napi_create_function(
    env,
    "memAddress",
    NAPI_AUTO_LENGTH,
    napi_memAddress,
    NULL,
    &napi_function);
  if ( status != napi_ok) {
    napi_throw_error(env, NULL, "napi_create_function() failed!");
  }

  // Set memAddress function in exports
  status = napi_set_named_property(
    env,
    exports,
    "memAddress",
    napi_function);
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "napi_set_named_property() failed!");
  }

  return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init);

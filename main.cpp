// Header files
#include <cstring>
#include <new>
#include <node_api.h>
#include <tuple>
#include <vector>

using namespace std;


// SMAZ namespace
namespace Smaz {

	// Header files
	#include "./SMAZ-NPM-Package-master/main.cpp"
}


// Constants

// Operation failed
static napi_value OPERATION_FAILED;


// Function prototypes

// Compress
static napi_value compress(napi_env environment, napi_callback_info arguments);

// Decompress
static napi_value decompress(napi_env environment, napi_callback_info arguments);

// Uint8 array to buffer
static tuple<uint8_t *, size_t, bool> uint8ArrayToBuffer(napi_env environment, napi_value uint8Array);

// Buffer to uint8 array
static napi_value bufferToUint8Array(napi_env environment, uint8_t *data, size_t size);


// Main function

// Initialize module
NAPI_MODULE_INIT() {

	// Check if initializing operation failed failed
	if(napi_get_null(env, &OPERATION_FAILED) != napi_ok) {
	
		// Return nothing
		return nullptr;
	}

	// Check if creating compress property failed
	napi_value temp;
	if(napi_create_function(env, nullptr, 0, compress, nullptr, &temp) != napi_ok || napi_set_named_property(env, exports, "compress", temp) != napi_ok) {
	
		// Return nothing
		return nullptr;
	}
	
	// Check if creating decompress property failed
	if(napi_create_function(env, nullptr, 0, decompress, nullptr, &temp) != napi_ok || napi_set_named_property(env, exports, "decompress", temp) != napi_ok) {
	
		// Return nothing
		return nullptr;
	}
	
	// Check if creating operation failed property failed
	if(napi_set_named_property(env, exports, "OPERATION_FAILED", OPERATION_FAILED) != napi_ok) {
	
		// Return nothing
		return nullptr;
	}
	
	// Return exports
	return exports;
}


// Supporting function implementation

// Compress
napi_value compress(napi_env environment, napi_callback_info arguments) {

	// Check if not enough arguments were provided
	size_t argc = 1;
	vector<napi_value> argv(argc);
	if(napi_get_cb_info(environment, arguments, &argc, argv.data(), nullptr, nullptr) != napi_ok || argc != argv.size()) {
	
		// Return operation failed
		return OPERATION_FAILED;
	}
	
	// Check if getting input from arguments failed
	const tuple<uint8_t *, size_t, bool> input = uint8ArrayToBuffer(environment, argv[0]);
	if(!get<2>(input)) {
	
		// Return operation failed
		return OPERATION_FAILED;
	}
	
	// Check if getting compressed size failed
	const size_t compressedSize = Smaz::compressSize(get<0>(input), get<1>(input));
	if(compressedSize == Smaz::invalidSize()) {
	
		// Return operation failed
		return OPERATION_FAILED;
	}
	
	// Check if compressing input failed
	vector<uint8_t> result(compressedSize);
	if(!Smaz::compress(result.data(), result.size(), get<0>(input), get<1>(input))) {
	
		// Return operation failed
		return OPERATION_FAILED;
	}
	
	// Return result as a uint8 array
	return bufferToUint8Array(environment, result.data(), result.size());
}

// Decompress
napi_value decompress(napi_env environment, napi_callback_info arguments) {

	// Check if not enough arguments were provided
	size_t argc = 1;
	vector<napi_value> argv(argc);
	if(napi_get_cb_info(environment, arguments, &argc, argv.data(), nullptr, nullptr) != napi_ok || argc != argv.size()) {
	
		// Return operation failed
		return OPERATION_FAILED;
	}
	
	// Check if getting input from arguments failed
	const tuple<uint8_t *, size_t, bool> input = uint8ArrayToBuffer(environment, argv[0]);
	if(!get<2>(input)) {
	
		// Return operation failed
		return OPERATION_FAILED;
	}
	
	// Check if getting decompressed size failed
	const size_t decompressedSize = Smaz::decompressSize(get<0>(input), get<1>(input));
	if(decompressedSize == Smaz::invalidSize()) {
	
		// Return operation failed
		return OPERATION_FAILED;
	}
	
	// Check if decompressing input failed
	vector<uint8_t> result(decompressedSize);
	if(!Smaz::decompress(result.data(), result.size(), get<0>(input), get<1>(input))) {
	
		// Return operation failed
		return OPERATION_FAILED;
	}
	
	// Return result as a uint8 array
	return bufferToUint8Array(environment, result.data(), result.size());
}

// Uint8 array to buffer
tuple<uint8_t *, size_t, bool> uint8ArrayToBuffer(napi_env environment, napi_value uint8Array) {

	// Check if uint8 array isn't a typed array
	bool isTypedArray;
	if(napi_is_typedarray(environment, uint8Array, &isTypedArray) != napi_ok || !isTypedArray) {
	
		// Return failure
		return {nullptr, 0, false};
	}
	
	// Check if uint8 array isn't a uint8 array
	napi_typedarray_type type;
	size_t size;
	uint8_t *data;
	if(napi_get_typedarray_info(environment, uint8Array, &type, &size, reinterpret_cast<void **>(&data), nullptr, nullptr) != napi_ok || type != napi_uint8_array) {
	
		// Return failure
		return {nullptr, 0, false};
	}
	
	// Return data and size
	return {data, size, true};
}

// Buffer to uint8 array
napi_value bufferToUint8Array(napi_env environment, uint8_t *data, size_t size) {

	// Check if allocating memory for buffer failed
	uint8_t *buffer = new(nothrow) uint8_t[size];
	if(!buffer) {
	
		// Clear data
		memset(data, 0, size);
	
		// Return operation failed
		return OPERATION_FAILED;
	}
	
	// Check if allocating memory for size hint failed
	size_t *sizeHint = new(nothrow) size_t(size);
	if(!sizeHint) {
	
		// Clear data
		memset(data, 0, size);
	
		// Free memory
		delete [] buffer;
	
		// Return operation failed
		return OPERATION_FAILED;
	}
	
	// Copy data
	memcpy(buffer, data, size);
	
	// Clear data
	memset(data, 0, size);
	
	// Check if creating array buffer from data failed
	napi_value arrayBuffer;
	if(napi_create_external_arraybuffer(environment, buffer, size, [](napi_env environment, void *finalizeData, void *finalizeHint) {
	
		// Get buffer
		uint8_t *buffer = reinterpret_cast<uint8_t *>(finalizeData);
		
		// Get size hint
		const size_t *sizeHint = static_cast<size_t *>(finalizeHint);
		
		// Clear buffer
		memset(buffer, 0, *sizeHint);
		
		// Free memory
		delete [] buffer;
		delete sizeHint;
	
	}, sizeHint, &arrayBuffer) != napi_ok) {
	
		// Clear buffer
		memset(buffer, 0, size);
	
		// Free memory
		delete [] buffer;
		delete sizeHint;
	
		// Return operation failed
		return OPERATION_FAILED;
	}
	
	// Check if creating uint8 array from array buffer failed
	napi_value uint8Array;
	if(napi_create_typedarray(environment, napi_uint8_array, size, arrayBuffer, 0, &uint8Array) != napi_ok) {
	
		// Clear buffer
		memset(buffer, 0, size);
	
		// Free memory
		delete [] buffer;
		delete sizeHint;
	
		// Return operation failed
		return OPERATION_FAILED;
	}
	
	// Return uint8 array
	return uint8Array;
}

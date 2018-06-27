#include <stdlib.h>

#include <node_api.h>
#include <pgb/debug.h>
#include <pgb/debugger/debugger.h>
#include <pgb/device/device.h>
#include <pgb/utils.h>

#include "napi_helper.h"

struct pgb_register_set {
	uint16_t value;
	const char *name;
};

static
struct device device;

static
napi_value napi_device_init(napi_env env, napi_callback_info info)
{
	int c_ret;
	size_t argc;
	napi_value argv[1], result;
	napi_status napi_ret;
	char *decoder_type;

	argc = 1;
	decoder_type = NULL;

	napi_ret = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
	OK_OR_RAISE_EXCEPTION(env, napi_ret == napi_ok, "Failed to parse arguments");

	napi_ret = napi_helper_get_string(env, info, argv[0], &decoder_type);
	OK_OR_RAISE_EXCEPTION(env, napi_ret == napi_ok, "Failed to convert argument to string");

	c_ret = device_init(&device, decoder_type);
	OK_OR_RAISE_EXCEPTION(env, c_ret == 0, "Failed to initialize device");

	free(decoder_type);

	napi_ret = napi_create_int32(env, 0, &result);
	OK_OR_RAISE_EXCEPTION(env, napi_ret == napi_ok, "Failed to create result");

	return result;
}

static
napi_value napi_device_reset(napi_env env, napi_callback_info info)
{
	int c_ret = 0;
	size_t argc = 2;
	napi_value argv[2], result;
	napi_status napi_ret;
	char *boot_rom_path, *decoder_type;

	decoder_type = NULL;
	boot_rom_path = NULL;

	napi_ret = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
	OK_OR_RAISE_EXCEPTION(env, napi_ret == napi_ok, "Failed to parse arguments");

	napi_ret = napi_helper_get_string(env, info, argv[0], &decoder_type);
	OK_OR_GOTO(napi_ret == napi_ok, free_and_exit);

	napi_ret = napi_helper_get_string(env, info, argv[1], &boot_rom_path);
	OK_OR_GOTO(napi_ret == napi_ok, free_and_exit);

	c_ret = device_reset_system(&device, decoder_type, boot_rom_path);
	OK_OR_GOTO(c_ret == 0, free_and_exit);

	napi_ret = napi_create_int32(env, 0, &result);
	OK_OR_RAISE_EXCEPTION(env, napi_ret == napi_ok, "Failed to create result");

free_and_exit:
	free(decoder_type);
	free(boot_rom_path);
	OK_OR_RAISE_EXCEPTION(env, napi_ret == napi_ok && c_ret == 0, "Failed to reset device");

	return result;
}

static
napi_value napi_device_load_image(napi_env env, napi_callback_info info)
{
	size_t argc;
	napi_value argv[1], result;
	napi_status napi_ret, c_ret;
	char *bootloader_rom_path;

	argc = 1;
	bootloader_rom_path = NULL;

	napi_ret = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
	OK_OR_RAISE_EXCEPTION(env, napi_ret == napi_ok, "Failed to parse arguments");

	napi_ret = napi_helper_get_string(env, info, argv[0], &bootloader_rom_path);
	OK_OR_RAISE_EXCEPTION(env, napi_ret == napi_ok, "Failed to convert argument to string");

	c_ret = device_load_image_from_file(&device, bootloader_rom_path);
	OK_OR_RAISE_EXCEPTION(env, c_ret == 0, "Failed to load bootloader image");

	free(bootloader_rom_path);

	napi_ret = napi_create_int32(env, 0, &result);
	OK_OR_RAISE_EXCEPTION(env, napi_ret == napi_ok, "Failed to create result");

	return result;
}

static
napi_value napi_device_cpu_read_registers(napi_env env, napi_callback_info info)
{
	size_t i;
	napi_value reg_value, result;
	napi_status napi_ret;

	struct pgb_register_set register_values[] = {
		{device.cpu.registers.af, "af"},
		{device.cpu.registers.bc, "bc"},
		{device.cpu.registers.de, "de"},
		{device.cpu.registers.hl, "hl"},
		{device.cpu.registers.pc, "pc"},
		{device.cpu.registers.sp, "sp"}
	};

	napi_ret = napi_create_object(env, &result);
	OK_OR_RAISE_EXCEPTION(env, napi_ret == napi_ok, "Failed to allocate result object");

	for (i = 0; i < ARRAY_SIZE(register_values); i++) {
		napi_ret = napi_create_uint32(env, register_values[i].value, &reg_value);
		OK_OR_RAISE_EXCEPTION(env, napi_ret == napi_ok, "Failed to create register value");

		napi_ret = napi_set_named_property(env, result, register_values[i].name, reg_value);
		OK_OR_RAISE_EXCEPTION(env, napi_ret == napi_ok, "Failed to set register value");
	}

	return result;
}

static
napi_value napi_device_cpu_step(napi_env env, napi_callback_info info)
{
	int c_ret;
	size_t stepped_instructions;
	napi_value result;
	napi_status napi_ret;

	c_ret = cpu_step(&device, 1, &stepped_instructions);
	OK_OR_RAISE_EXCEPTION(env, c_ret == napi_ok, "Failed to step cpu");

	napi_ret = napi_create_int32(env, 0, &result);
	OK_OR_RAISE_EXCEPTION(env, napi_ret == napi_ok, "Failed to create result");

	return result;
}

static
napi_value napi_device_mmu_read_region(napi_env env, napi_callback_info info)
{
	int c_ret;
	napi_value result, byte_value;
	napi_value argv[2];
	napi_status napi_ret;
	size_t i, region_size, argc = 2;
	uint8_t *region;
	uint32_t base_address, rs_convert;

	napi_ret = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
	OK_OR_RAISE_EXCEPTION(env, napi_ret == napi_ok, "Failed to parse arguments");

	napi_ret = napi_get_value_uint32(env, argv[0], &base_address);
	OK_OR_RAISE_EXCEPTION(env, napi_ret == napi_ok, "Failed to convert argument 0");

	napi_ret = napi_get_value_uint32(env, argv[1], &rs_convert);
	OK_OR_RAISE_EXCEPTION(env, napi_ret == napi_ok, "Failed to convert argument 1");

	region_size = rs_convert;

	region = malloc(region_size);
	OK_OR_RAISE_EXCEPTION(env, region != NULL, "Failed to allocate memory");

	c_ret = mmu_read_region(&device.mmu, (uint16_t)base_address, region, &region_size);
	OK_OR_RAISE_EXCEPTION(env, c_ret == 0, "Failed to read CPU memory region");

	napi_ret = napi_create_array(env, &result);
	OK_OR_GOTO(napi_ret == napi_ok, free_and_exit);

	for (i = 0; i < region_size; i++) {
		napi_ret = napi_create_uint32(env, region[i], &byte_value);
		OK_OR_GOTO(napi_ret == napi_ok, free_and_exit);

		napi_ret = napi_set_element(env, result, i, byte_value);
		OK_OR_GOTO(napi_ret == napi_ok, free_and_exit);
	}

free_and_exit:
	free(region);
	OK_OR_RAISE_EXCEPTION(env, napi_ret == napi_ok, "Failed to read memory region");

	return result;
}

static
napi_value napi_device_mmu_write_byte(napi_env env, napi_callback_info info)
{
	int c_ret;
	size_t argc = 2;
	napi_value argv[2], result;
	napi_status napi_ret;
	uint32_t address, value;

	napi_ret = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
	OK_OR_RAISE_EXCEPTION(env, napi_ret == napi_ok, "Failed to parse arguments");

	napi_ret = napi_get_value_uint32(env, argv[0], &address);
	OK_OR_RAISE_EXCEPTION(env, napi_ret == napi_ok, "Failed to convert argument");

	napi_ret = napi_get_value_uint32(env, argv[1], &value);
	OK_OR_RAISE_EXCEPTION(env, napi_ret == napi_ok, "Failed to convert argument");

	c_ret = mmu_write_byte(&device.mmu, (uint16_t)address, (uint8_t)value);
	OK_OR_RAISE_EXCEPTION(env, c_ret == 0, "Failed to write byte to address");

	napi_ret = napi_create_int32(env, 0, &result);
	OK_OR_RAISE_EXCEPTION(env, napi_ret == napi_ok, "Failed to create result");

	return result;
}

static
napi_value napi_debugger_disasm(napi_env env, napi_callback_info info)
{
	int c_ret;
	size_t i, j, argc = 1;
	napi_value result, argv[1], entry, value, raw_data;
	napi_status napi_ret;
	uint32_t num_instructions;

	napi_ret = napi_get_cb_info(env, info, &argc, argv, NULL, NULL);
	OK_OR_RAISE_EXCEPTION(env, napi_ret == napi_ok, "Failed to parse arguments");

	napi_ret = napi_get_value_uint32(env, argv[0], &num_instructions);
	OK_OR_RAISE_EXCEPTION(env, napi_ret == napi_ok, "Failed to convert argument");

	struct debugger_info disasm_info[num_instructions];

	c_ret = debugger_fetch_instructions(&device, disasm_info, num_instructions);
	OK_OR_GOTO(c_ret == 0, free_and_exit);

	napi_ret = napi_create_array(env, &result);
	OK_OR_GOTO(napi_ret == napi_ok, free_and_exit);

	for (i = 0; i < num_instructions; i++) {
		napi_ret = napi_create_object(env, &entry);
		OK_OR_GOTO(napi_ret == napi_ok, free_and_exit);

		napi_ret = napi_create_string_utf8(env, disasm_info[i].assembly, NAPI_AUTO_LENGTH, &value);
		OK_OR_GOTO(napi_ret == napi_ok, free_and_exit);

		napi_ret = napi_set_named_property(env, entry, "assembly", value);
		OK_OR_GOTO(napi_ret == napi_ok, free_and_exit);

		napi_ret = napi_create_uint32(env, disasm_info[i].address, &value);
		OK_OR_GOTO(napi_ret == napi_ok, free_and_exit);

		napi_ret = napi_set_named_property(env, entry, "address", value);
		OK_OR_GOTO(napi_ret == napi_ok, free_and_exit);

		napi_ret = napi_create_array(env, &raw_data);
		OK_OR_GOTO(napi_ret == napi_ok, free_and_exit);

		for (j = 0; j < disasm_info[i].num_bytes; j++) {
			napi_ret = napi_create_uint32(env, disasm_info[i].raw_data[j], &value);
			OK_OR_GOTO(napi_ret == napi_ok, free_and_exit);

			napi_ret = napi_set_element(env, raw_data, j, value);
			OK_OR_GOTO(napi_ret == napi_ok, free_and_exit);
		}

		napi_ret = napi_set_named_property(env, entry, "raw_data", raw_data);
		OK_OR_GOTO(napi_ret == napi_ok, free_and_exit);

		if (disasm_info[i].comments.a != NULL) {
			napi_ret = napi_create_string_utf8(env, disasm_info[i].comments.a, NAPI_AUTO_LENGTH, &value);
			OK_OR_GOTO(napi_ret == napi_ok, free_and_exit);

			napi_ret = napi_set_named_property(env, entry, "comment_a", value);
			OK_OR_GOTO(napi_ret == napi_ok, free_and_exit);
		}

		if (disasm_info[i].comments.b != NULL) {
			napi_ret = napi_create_string_utf8(env, disasm_info[i].comments.b, NAPI_AUTO_LENGTH, &value);
			OK_OR_GOTO(napi_ret == napi_ok, free_and_exit);

			napi_ret = napi_set_named_property(env, entry, "comment_b", value);
			OK_OR_GOTO(napi_ret == napi_ok, free_and_exit);
		}

		napi_ret = napi_set_element(env, result, i, entry);
		OK_OR_GOTO(napi_ret == napi_ok, free_and_exit);
	}

free_and_exit:
	// free_debugger_info(disasm_info, num_instructions);
	OK_OR_RAISE_EXCEPTION(env, c_ret == 0 && napi_ret == napi_ok, "Failed to dissasm location");

	return result;
}

static
struct exported_function {
	napi_value (*func_ptr)(napi_env env, napi_callback_info info);
	const char *func_name;
} exported_functions[] = {
	{napi_device_init,               "device_init"},
	{napi_device_reset,              "device_reset"},
	{napi_device_load_image,         "device_load_image"},
	{napi_device_cpu_read_registers, "device_cpu_read_registers"},
	{napi_device_cpu_step,           "device_cpu_step"},
	{napi_device_mmu_read_region,    "device_mmu_read_region"},
	{napi_device_mmu_write_byte,     "device_mmu_write_byte"},
	{napi_debugger_disasm,           "debugger_disasm"}
};

napi_value Init(napi_env env, napi_value exports)
{
	size_t i;
	napi_status ret;

	for (i = 0; i < ARRAY_SIZE(exported_functions); i++) {
		ret = napi_helper_declare_method(env, exports, exported_functions[i].func_ptr, exported_functions[i].func_name);
		OK_OR_RAISE_EXCEPTION(env, ret == napi_ok, "Unable to wrap native function");
	}

	return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
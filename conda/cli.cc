﻿#include <cstddef>
#include <vector>

#include <Windows.h>
#include <shellapi.h>

#include "common/types.h"
#include "common/debug.h"
#include "common/strings.h"
#include "common/console.h"
#include "common/log.h"
#include "common/console_logger.h"
#include "common/file_helpers.h"
#include "common/scoped_function.h"

#include "iso9660/iso_file.h"
#include "hd/hd_file.h"

set_log_channel("conda")

using namespace common;

using string_list = std::vector<std::string>;
using string_list_arg = const std::vector<std::string>&;
using cmd_pfn = std::add_pointer_t<bool(string_list_arg)>;

using iso_file_entry = iso9660::file::file_entry;
using iso_file_list = iso9660::file::file_list;
using iso_file = iso9660::file;

struct cmd_info
{
  std::string_view name;
  std::string_view args;
  std::string_view short_description;
  std::string_view long_description;
};

static constexpr std::array command_info_list =
{
  cmd_info
  {
    .name              = "help",
    .args              = "[command]",
    .short_description = "Command help",
    .long_description  = "  Prints help and given an optional command name prints command usage"
  },
  cmd_info
  {
    .name              = "extract-iso",
    .args              = "iso_file_path (dir|file|hdx) entry_path [output_directory_path]",
    .short_description = "Extract an iso image",
    .long_description  = "  Extracts the iso at given entry_path and outputs it's contents to optional output_path.\n"
                         "  If no output_path is specified one will be created in the folder that contains the executable."
  },
  cmd_info
  {
    .name              = "extract-pak",
    .args              = "pak_file_path [output_directory_path]",
    .short_description = "Extract a pak archive",
    .long_description  = "  Extracts a pak archive given pak_file_path\n"
                         "  If no output_directory_path is specified the name and location of the pak file is used.\n\n"
                         "  Valid archive extensions: .pak, .chr, .efp, .ipk, .mpk, .pcp, .sky, .snd"
  },
  cmd_info
  {
    .name              = "extract-img",
    .args              = "img_file_path [output_directory_path]",
    .short_description = "Extract an img archive",
    .long_description  = "  Extract an img archive given img_file_path\n"
                         "  If no output_directory_path is specified the name and location of the img file is used.\n\n"
                         "  Valid archive extensions: .img"
  },
  cmd_info
  {
    .name              = "decompile-script",
    .args              = "script_file_path [output_file_path]",
    .short_description = "Decompile an RS script",
    .long_description  = "  Decompiles an RS script given script_file_path\n"
                         "  If no output_file_path is provided the name and location of the script is used and the file is used."
  },
};

static std::string s_program_name{ };

static void print_program_header()
{
  console::writeln("  ____                _______             __");
  console::writeln(" / __ \\___  ___ ___  / ___/ /__  __ _____/ /");
  console::writeln("/ /_/ / _ \\/ -_) _ \\/ /__/ / _ \\/ // / _  / ");
  console::writeln("\\____/ .__/\\__/_//_/\\___/_/\\___/\\_,_/\\_,_/  ");
  console::writeln("    /_/                                     ");
  console::write("\n");
}

static void print_command_help(std::string_view command_name)
{
  const auto itr = std::find_if(command_info_list.begin(), command_info_list.end(), [&](const cmd_info& entry) {
    return entry.name == command_name;
  });

  if (itr == command_info_list.end())
  {
    console::writeln_format("Unknown command {}", command_name);
    return;
  }

  console::writeln_format("{} {} {}", s_program_name, itr->name, itr->args);
  console::write("\n");
  console::writeln_format("{}", itr->long_description);
}

static void print_program_help()
{
  console::writeln_format("{} <command> [command_options]", s_program_name);
  console::write("\n");

  console::writeln("commands:");

  for (const auto& arg : command_info_list)
    console::writeln_format("  {:<20}{:<20}", arg.name, arg.short_description);
}

static std::optional<iso9660::file::file_entry> search_iso_for_file_entry(std::unique_ptr<iso_file>& iso, std::string_view iso_relative_path)
{
  const auto file_list = iso->files_for_directory(file_helpers::parent_directory(iso_relative_path));

  const auto itr = std::find_if(file_list.begin(), file_list.end(), [&](const iso_file_entry& entry) {
    if (entry.path == iso_relative_path)
      return true;

    return false;
  });

  if (itr == file_list.end())
    return std::nullopt;

  return *itr;
}

static bool extract_file_entry_from_iso(std::unique_ptr<iso_file>& iso, const iso_file_entry& entry, std::string_view output_path)
{
  const auto utf8_path = strings::sjis_to_utf8_or_panic(entry.path);
  const auto full_output_path = file_helpers::append(output_path, utf8_path);
  const auto parent_directory_path = file_helpers::parent_directory(full_output_path);

  if (!file_helpers::create_directories(parent_directory_path))
  {
    log_error("Failed to create output directory {}", parent_directory_path);

    return false;
  }

  std::unique_ptr<data_stream_base> file_stream = file_stream::open(full_output_path, "wb");

  if (!file_stream)
  {
    log_error("Failed to open output file {}", full_output_path);

    return false;
  }

  if (!iso->copy_file_entry_to_stream(entry, file_stream.get()))
  {
    log_error("Failed to copy file entry to output file {}", full_output_path);

    return false;
  }

  return true;
}

static bool extract_file_from_iso(std::unique_ptr<iso_file>& iso, std::string_view iso_relative_path, std::string_view output_directory)
{
  const auto file = search_iso_for_file_entry(iso, file_helpers::native_path(iso_relative_path));

  if (!file)
    return false;

  return extract_file_entry_from_iso(iso, *file, output_directory);
}


static bool extract_directory_from_iso(std::unique_ptr<iso_file>& iso, std::string_view iso_relative_path, std::string_view output_directory)
{
  for (const auto& file : iso->files_for_directory(file_helpers::native_path(iso_relative_path)))
  {
    if (!extract_file_entry_from_iso(iso, file, output_directory))
    {
      log_warn("Unable to extract file from {} ... skipping", iso_relative_path);

      continue;
    }
  }

  return true;
}

static bool extract_hdx_file_from_iso(std::unique_ptr<iso_file>& iso, std::string_view iso_relative_path, std::string_view output_directory)
{
  const auto basename = file_helpers::basename(iso_relative_path);
  const auto parent_directory = file_helpers::parent_directory(iso_relative_path);

  const auto archive_path = file_helpers::append(parent_directory, std::string{ basename } + ".DAT");

  auto descriptor_file_entry = search_iso_for_file_entry(iso, iso_relative_path);
  auto archive_file_entry = search_iso_for_file_entry(iso, archive_path);

  if (!descriptor_file_entry || !archive_file_entry)
    return false;

  usize current_byte = 0;
  while (current_byte < descriptor_file_entry->total_bytes)
  {
    iso->seek_to_logical_block(descriptor_file_entry->logical_block_address);
    iso->seek_relative_to_logical_block(current_byte);

    hdx::hd3_file_descriptor desc;
    if (!iso->copy_bytes_to_buffer(&desc, sizeof(hdx::hd3_file_descriptor)))
      return false;

    if (desc.size == 0 && desc.logical_block_size == 0 && desc.logical_block_offset == 0)
      break;

    current_byte += sizeof(hdx::hd3_file_descriptor);

    iso->seek_relative_to_logical_block(desc.name_location);

    std::array<u8, file_stream::block_size> temp;
    if (!iso->copy_bytes_to_buffer(temp.data(), temp.size()))
      continue;

    char* str_of_unknown_size = std::bit_cast<char*>(temp.data());
    const auto name_size = strnlen_s(str_of_unknown_size, temp.size());

    if (name_size >= temp.size())
      continue;

    auto name = strings::sjis_to_utf8_or_panic({str_of_unknown_size, name_size});

    const auto output_path = file_helpers::append(output_directory, name);
    const auto parent_directory_path = file_helpers::parent_directory(output_path);

    if (!file_helpers::create_directories(parent_directory_path))
    {
      log_warn("Failed to create output directory {}... skipping file", parent_directory_path);

      continue;
    }

    iso->seek_to_logical_block(archive_file_entry->logical_block_address + desc.logical_block_offset);

    std::unique_ptr<data_stream_base> output_file_stream = file_stream::open(output_path, "wb");

    if (!output_file_stream)
    {
      log_warn("Failed to open output file stream {}... skipping file", output_path);

      continue;
    }

    if (!iso->copy_bytes_to_stream(output_file_stream.get(), desc.size))
    {
      log_warn("Failed to copy {} bytes to output file stream... skipping file", desc.size);

      continue;
    }
  }

  return true;
}

static bool cmd_help(string_list_arg args)
{
  if (args.size() == 1)
  {
    print_command_help(args[0]);

    return true;
  }

  // print the program help but return success
  print_program_help();

  return true;
}

static bool cmd_extract_iso(string_list_arg args)
{
  if (args.size() != 3 && args.size() != 4)
  {
    print_command_help("extract-iso");

    return false;
  }

  auto output_path = file_helpers::append(file_helpers::get_application_directory(), "game_data");

  const std::string_view iso_file_path = args[0];
  const std::string_view entry_type = args[1];
  const std::string_view file_entry_path = args[2];

  if (args.size() == 4)
    output_path = args[3];

  std::unique_ptr<iso_file> iso = iso_file::open(iso_file_path);

  if (!iso)
    return false;

  if (!file_helpers::create_directory(output_path))
    return false;

  if (entry_type == "file")
    return extract_file_from_iso(iso, file_entry_path, output_path);

  if (entry_type == "dir")
    return extract_directory_from_iso(iso, file_entry_path, output_path);

  if (entry_type == "hdx")
    return extract_hdx_file_from_iso(iso, file_entry_path, output_path);

  print_command_help("extract-iso");

  return false;
}

static bool cmd_extract_pak(MAYBE_UNUSED string_list_arg args)
{
  return true;
}

static bool cmd_extract_img(MAYBE_UNUSED string_list_arg args)
{
  return true;
}

static bool cmd_decompile_script(MAYBE_UNUSED string_list_arg args)
{
  return true;
}

static constexpr std::array command_function_list =
{
  cmd_pfn{ cmd_help },
  cmd_pfn{ cmd_extract_iso },
  cmd_pfn{ cmd_extract_pak },
  cmd_pfn{ cmd_extract_img },
  cmd_pfn{ cmd_decompile_script },
};

static std::vector<std::string> get_cmd_line_utf8()
{
  // we're using this instead of the lpCmdLine passed to winmain
  // the version passed to winmain does not include the exe name
  const LPWSTR lpCmdLine = GetCommandLineW();

  int count;
  LPWSTR* szArglist = CommandLineToArgvW(lpCmdLine, &count);

  std::vector<std::string> utf8_args;

  // convert everything to UTF-8
  for (int i = 0; i < count; ++i)
  {
    auto arg = common::strings::wstring_to_utf8(szArglist[i]);

    if (arg)
      utf8_args.push_back(std::move(*arg));
  }

  // > the calling application must free the memory used by the argument list when it is no longer needed.
  LocalFree(szArglist);

  return utf8_args;
}

static bool initialize_console_logger()
{
  if (!console::initialize())
    return false;

  if (!log::console_logger::initialize())
    return false;

  return true;
}

INT WINAPI WinMain(_In_ HINSTANCE /*hInstance*/, _In_opt_ HINSTANCE /*hPrevInstance*/,
  _In_ LPSTR /*lpCmdLine*/, _In_ INT /*nCmdShow*/)

{
  // we need the console for this
  if (!initialize_console_logger())
    return EXIT_FAILURE;

  scoped_function cleanup([&]() {
    log::console_logger::shutdown();
    console::shutdown();
  });

  print_program_header();

  auto arg_list = get_cmd_line_utf8();

  // set the application directory to the parent of this app
  file_helpers::set_application_directory(file_helpers::parent_directory(arg_list.front()));

  // get the program name as the basename of the app name
  s_program_name = file_helpers::basename(arg_list.front());

  arg_list.erase(arg_list.begin());

  if (arg_list.size() == 0)
  {
    print_program_help();

    return EXIT_FAILURE;
  }

  for (usize i = 0; i < command_info_list.size(); i++)
  {
    if (command_info_list[i].name != arg_list.front())
      continue;

    arg_list.erase(arg_list.begin());

    if (!command_function_list[i](arg_list))
      return EXIT_FAILURE;

    return EXIT_SUCCESS;
  }

  print_program_help();

  return EXIT_FAILURE;
}

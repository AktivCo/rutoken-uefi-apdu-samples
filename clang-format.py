#!/usr/bin/env python2
# -*- coding: utf-8 -*-
from argparse import ArgumentParser
from sys import argv, stderr, stdout
from shutil import copyfileobj
from re import compile, sub
from os import walk, path, devnull
from os import name as os_name
from fnmatch import filter
from subprocess import check_output, CalledProcessError, call, STDOUT, Popen, PIPE

extension = ""
if os_name == "nt":
    extension = ".exe"


def print_warning(text=None):
    stderr.write("WARNING : {0}\n".format(text))


def error(text, exit_code):
    stderr.write("ERROR : {0}\n".format(text))
    exit(exit_code)


def cat_file(path_to_file):
    with open(path_to_file, "r") as hfile:
        copyfileobj(hfile, stdout)


def console(command=None, call_func=False, silent=False):
    output = None
    try:
        if call_func:
            if silent:
                output = call(command, shell=True, stdout=open(devnull, "wb"), stderr=STDOUT)
            else:
                output = call(command, shell=True)
        else:
            output = check_output(command, shell=True)
    except CalledProcessError as e:
        error(e, e.returncode)
    return output


def file_in_dirs(path_to_file=None, dirs=None):
    if not dirs:
        return None

    while path_to_file != execute_path:
        path_to_file = path.dirname(path_to_file)
        if path_to_file in dirs:
            return path_to_file
    return None

def extension_by_language(language):
    if language == "cpp":
        return ".cpp"
    elif language == "objc":
        return ".m"
    else:
        error("Invalid language specification in git-attributes: {0}".format(language), 1)

def format_file(path_to_file=None):
    relative_path = path.relpath(path_to_file, path.commonprefix([path_to_file, execute_path]))

    check_attr = console("git check-attr clang-format -- {0}".format(relative_path))
    lang = compile(": clang-format: (.*)\n").findall(check_attr)[0]
    if lang == "unset" or lang == "unspecified":
        return 0

    '''
    clang-format uses file extension to select language from config by default.
    .h files could be used in Objective-C code, so if not using -assume-filename option
    C++ config will be used. But -assume-filename is ignored if we pass filename, so
    we read file and pass its contents using pipe.
    '''
    source_extension = extension_by_language(lang)

    with open(path_to_file, 'rb') as f:
        contents = f.read()
    clang_format = Popen(["clang-format" + extension, "-assume-filename=dummy" + source_extension, "-style=file"],
                         stdin=PIPE, stdout=PIPE)
    formatted_contents = clang_format.communicate(contents)[0]
    retcode = clang_format.poll();
    if retcode != 0:
        error("Error during the call to clang-format", 1)

    with open(path_to_file, 'wb') as f:
        f.write(formatted_contents);

def filter_list(data_list):
    return [element for element in data_list if element != ""]


parser = ArgumentParser(description='Format files in git using clang-format.',
                        epilog='By default it formats files changed after HEAD.')
group = parser.add_mutually_exclusive_group()
group.add_argument('path', nargs='?', help='path to files, may be file or directory')
group.add_argument('-p', '--previous', action='store_true', help='format files changed after HEAD^')

args = parser.parse_args()

execute_path = path.dirname(path.realpath(__file__))

if args.path:
    abspath_dst = path.abspath(args.path)
    if not path.exists(abspath_dst):
        error("{0} file does not exist".format(abspath_dst), 1)
    if path.isfile(abspath_dst):
        format_file(abspath_dst)
    elif path.isdir(abspath_dst):
        files = []
        for root, dirnames, filenames in walk(abspath_dst):
            for f in filenames:
                files.append(path.join(root, f))

        for file in files:
            if "clang-format" in file:
                continue
            format_file(file)
else:
    revision = 'HEAD'
    if args.previous:
        revision = 'HEAD^'

    files = console('git diff {} --name-only --diff-filter=d'.format(revision)).split('\n')
    files = filter_list(files)

    for f in map(lambda p: path.abspath(p), files):
        format_file(f)

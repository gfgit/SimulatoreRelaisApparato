#!/usr/bin/python3

import os
import re
import sys
import time
import datetime
#import shutil

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))


def print_error(msg, filename):
    print(filename + ': Error: ' + msg)


def print_fixed(msg, filename):
    print(filename + ': Fixed: ' + msg)


def check_license(text, filepath, fix=False):
    m = re.search(r'^/\*\*\n.+\n \*/\n\n', text, re.DOTALL)
    if m is None:
        if fix:
            is_test = (os.path.sep + 'test' + os.path.sep) in filepath
            text = '''/**
 * ''' + filepath + '''
 *
 * This file is part of the Simulatore Relais Apparato ''' + ('test suite' if is_test else 'source code') + '''.
 *
 * Copyright (C) ''' + str(datetime.datetime.now().year) + ''' Filippo Gentile
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

''' + text
            print_fixed('license block', filepath)
        else:
            print_error('can\'t find license block', filepath)

    return text


def check_include_guard(text, filepath, fix=False):
    guard = 'TRAINTASTIC_' + filepath.replace(os.sep, '_').replace('.', '_').upper().replace('_SRC_', '_')
    m = re.search(r'^([^#]*)#ifndef ([0-9A-Z_]+)\n#define \2\n', text, re.MULTILINE)
    if m is None:
        print_error('can\'t find include guard', filepath)
    elif m[2] != guard:
        if fix:
            text = re.sub(r'^([^#]*)#ifndef ([0-9A-Z_]+)\n#define \2\n', '\\1#ifndef ' + guard + '\n#define ' + guard + '\n', text, re.MULTILINE)
            print_fixed('include guard', filepath)
        else:
            print_error('invalid include guard, got ' + m[2] + ', expected ' + guard, filepath)

    return text


def check_sources(path, fix=False):
    for subdir, dirs, files in os.walk(path):
        for filename in files:
            is_hpp = filename.endswith('.hpp') or filename.endswith('.h')
            is_cpp = filename.endswith('.cpp')
            if is_hpp or is_cpp:
                filepath = os.path.join(subdir, filename)
                with open(filepath, 'r', encoding='utf8') as f:
                    text = text_org = f.read()
                filepath_project = os.path.relpath(filepath, PROJECT_ROOT)

                if is_hpp or is_cpp:
                    text = check_license(text, filepath_project, fix)

                    # Check file path in header comment
                    text_lines = text.splitlines()

                    new_path_str = ' * ' + filepath_project
                    old_path_str = text_lines[1]

                    if old_path_str != new_path_str:
                        text_lines[1] = new_path_str
                        text = '\n'.join(text_lines)
                        print(filepath + ': Fixed: path')

                    if not text.endswith('\n'):
                        text = text + '\n'
                        print(filepath + ': Fixed: newline at end')

                #if is_hpp:
                #    text = check_include_guard(text, filepath_project, fix)

                if fix and text != text_org:
                    #os.rename(filepath, filepath + '~' + str(int(time.time())))
                    os.remove(filepath)
                    with open(filepath, 'w+') as f:
                        f.write(text)


if __name__ == "__main__":
    SOURCE_DIRS = ['src']

    fix = True

    for source_dir in SOURCE_DIRS:
        check_sources(os.path.abspath(os.path.join(PROJECT_ROOT, source_dir)), fix)

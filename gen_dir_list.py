# -*- coding: utf-8 -*-

import os
import sys
import struct

USAGE = """
python gen_dir_list.py src_path [dst_path]
"""

def usage(): print USAGE


def format_path(path):
	if len(path) == 0: return "./"

	path = path.replace('\\', '/')
	if path[-1] != '/': path += '/'
	return path


def list_all_files(path):
	ret = []
	files = os.listdir(path)
	files.sort()
	for fname in files:
		fpath = path + fname
		if os.path.isdir(fpath):
			fpath += '/'
			child_ret = list_all_files(fpath)
			ret.append( (fname, child_ret) )
		else:
			ret.append( (fname, None) )

	return ret


def output_dirs(path, handle):
	name, children = path

	handle.write( struct.pack("H", len(name)) )
	handle.write( name )
	nchild = 0xffff if children is None else len(children)
	handle.write( struct.pack("H", nchild) )

	if children is not None:
		for child in children:
			output_dirs(child, handle)
	return


def gen_dir_list(src_path, dst_path):
	src_path = format_path(src_path)
	dst_path = format_path(dst_path)

	print "collect files: ", src_path
	paths = list_all_files(src_path)

	filename = dst_path + "ora.dir"
	print "write fo file: ", filename
	handle = open(filename, "wb")
	output_dirs((".", paths), handle)
	handle.close()



def main():
	if len(sys.argv) < 2:
		return usage()

	src_path = sys.argv[1]
	dst_path = sys.argv[2] if len(sys.argv) > 2 else src_path
	gen_dir_list(src_path, dst_path)

if __name__ == "__main__":
	main()

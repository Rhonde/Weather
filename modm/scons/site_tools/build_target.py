# -*- coding: utf-8 -*-
#
# Copyright (c) 2018, Niklas Hauser
#
# This file is part of the modm project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
# -----------------------------------------------------------------------------

def build_target(env, sources):
	# Building application
	program = env.Program(target=env["CONFIG_PROJECT_NAME"]+".elf", source=sources)

	# SCons functions
	env.Alias("cmakewrapper", env.CMakeWrapper())
	env.Alias("qtcreator", env.QtCreatorProject(sources))
	env.Alias("symbols", env.Symbols(program))
	env.Alias("listing", env.Listing(program))
	env.Alias("bin", env.Bin(program))
	env.Alias("size", env.Size(program))
	env.Alias("program", env.OpenOcd(program, commands=["modm_program $SOURCE"]))
	env.Alias("bmp", env.BlackMagicProbe(program))
	env.Alias("gdb", env.OpenOcdGdb(program))
	env.Alias("build", [program, "listing"])
	env.Alias("all", ["build", "size"])
	env.Default("all")


def generate(env, **kw):
	env.AddMethod(build_target, "BuildTarget")

def exists(env):
	return True
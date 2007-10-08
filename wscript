#!/usr/bin/env python

VERSION='0.0.4'
APPNAME='fama'

srcdir = '.'
blddir = '_build_'

def set_options(opt):
    opt.tool_options('compiler_cc')

def configure(conf):
    conf.check_tool('compiler_cc')

    conf.check_pkg('glib-2.0', destvar='GLIB', vnum='2.10.0')
    conf.check_pkg('libtapioca-client-glib-0.14', destvar='LIBTAPIOCA-CLIENT-GLIB', vnum='0.14.0.2') 
    conf.check_pkg('gobject-2.0', destvar='GOBJECT', vnum='2.10.0')
    conf.check_pkg('libreadline5', destvar='LIBREADLINE')
    conf.check_header('ncursesw/panel.h')
    conf.check_header('ncursesw/ncurses.h')

    conf.env['LIB_NCURSESW'] = "ncursesw"
    conf.env['LIBPATH_NCURSESW'] = '/usr/include'

    conf.env['LIB_PANEL'] = "panelw"
    conf.env['LIBPATH_PANEL'] = '/usr/include'

    conf.add_define('VERSION', VERSION)
    conf.add_define('MAJORMINOR', '.'.join(VERSION.split('.')[0:2]))
    
    conf.add_define('PACKAGE', APPNAME)

    conf.env.append_value('CCFLAGS', '-DHAVE_CONFIG_H')

    conf.write_config_header('config.h')
    conf.env.append_value('CCFLAGS', '-I/usr/include/ncursesw')

def build(bld):
    bld.add_subdirs('src')

def shutdown():
    import UnitTest, Object
    unittest = UnitTest.unit_test()
    #unittest.want_to_see_test_output = True
    unittest.run()
    unittest.print_results()

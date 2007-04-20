import os
from build_functions import CheckPKGConfig, CheckPKG, CheckPackages, SaveDictionary

SConsignFile()

env = Environment(ENV = os.environ)

# Variables

# [library_name, lirary_minimum_version]
libraries = [
             ['glib-2.0', '2.0.0'],
             ['gobject-2.0', '2.0.0'],
            ]

options_filename = 'build_options'

application_version_string = '0.0.1-pre-alpha'

# Get our configuration options:
opts = Options(options_filename)
opts.Add('PREFIX', 'Directory to install under', '/usr/local')
opts.Add('NCURSESW', 'Path to the ncursesw include path', '/usr/include/ncursesw')
opts.Add(BoolOption('CONFIGURE', 'Whether the build should be (re)configured', 'yes'))
opts.Add(BoolOption('DEBUG', 'Whether debugging information should be produced', 'no'))
opts.Update(env)
opts.Save(options_filename, env)

Help(opts.GenerateHelpText(env))

# Compiler options

if env['CC'] == 'gcc':

    env['CCFLAGS'] += ' -Wall'

    if env['DEBUG'] == True:
        env['CCFLAGS'] += ' -g'


# Configuration:

if env['CONFIGURE'] == True:
    conf = Configure (env, custom_tests = { 'CheckPKGConfig' : CheckPKGConfig,
                                            'CheckPKG' : CheckPKG })

    if not conf.CheckPKGConfig('0.15.0'):
         print 'pkg-config >= 0.15.0 not found.'
         Exit(1)

    if not CheckPackages (conf, libraries):
        Exit (1)

    env = conf.Finish()

    env['CONFIGURE'] = 'no' # If this point has been reached it's because the configuration was successful
    opts.Save(options_filename, env)

    # Configuration header file.
    env.config_header_vars = {
        # This is where you put all of your custom configuration values.
        'data_dir_prefix'            : os.path.join (env['PREFIX'], 'share', 'fama', ''),
        'version_string'             : application_version_string
    }
    SaveDictionary ('config_header_vars', env.config_header_vars)

# Now, build:

for library_name, library_version in libraries:
    env.ParseConfig ('pkg-config --cflags --libs ' + library_name)


# Append path to ncursesw directory


SConscript(['src/SConscript'], 'env options_filename')

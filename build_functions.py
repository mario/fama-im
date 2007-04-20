# Check pgk-config existence and its version
def CheckPKGConfig(context, version):
    context.Message( 'Checking for pkg-config... ' )
    ret = context.TryAction('pkg-config --atleast-pkgconfig-version=%s' % version)[0]
    context.Result( ret )
    return ret

# Check whether a given package is installed using pkg-config
def CheckPKG (context, library_name, library_version):
    context.Message ('Checking for %s >= %s...' % (library_name, library_version) )
    ret = context.TryAction ('pkg-config --print-errors --exists \'%s >= %s\'' % (library_name, library_version))[0]
    context.Result (ret)
    return ret

# Check a list of package names
def CheckPackages (conf, library_list):
    for library_name, library_version in library_list:
        if not conf.CheckPKG(library_name, library_version):
            return False
    return True

def BuildConfigHeader (target, source, env):

    for a_target, a_source in zip(target, source):
        print "Generating configuration header %s from %s..." % (a_target, a_source),
        config_h = file(str(a_target), "w")
        config_h_in = file(str(a_source), "r")
        config_h.write(config_h_in.read() % env.config_header_vars)
        config_h_in.close()
        config_h.close()
        print "done"

def SaveDictionary (filename, dic):
    dic_file = file (filename, 'w')

    for key, val in dic.items():
        dic_file.write ("%s - %s\n" % (key, val))

    dic_file.close ()

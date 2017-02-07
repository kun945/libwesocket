import os
import ycm_core

flags = [
    '-Wall'
    , '-Wextra'
    , '-Wno-long-long'
    , '-Wno-variadic-macros'
    , '-fexceptions'
    , '-undef'
    , '-DNDEBUG'
    , '-D_GNU_SOURCE'
    , '-DVERSION_MAJOR=0'
    , '-DVERSION_MINOR=0'
    , '-DBUILD_DATE=""'
    , '-DUSE_CLANG_COMPLETER'
    , '-std=c99'
    , '-x'
    , 'c'
    , '-isystem'
    , 'K:/cygwin/opt/userver/usr/include'
    , '-isystem'
    , 'K:/cygwin/opt/userver/usr/include/x86_64-linux-gnu'
    , '-isystem'
    , 'K:/cygwin/opt/userver/usr/local/include'
    , '-isystem'
    , 'K:/cygwin/opt/userver/usr/lib/gcc/x86_64-linux-gnu/4.8/include'
    , '-isystem'
    , 'K:/cygwin/opt/userver/usr/lib/gcc/x86_64-linux-gnu/4.8/include-fixed'
    , '-I'
    , 'z:/sources/ws/src'
    , '-I'
    , 'z:/sources/ws/include'
    , '-I'
    , '.'
]

compilation_database_folder = ''
if compilation_database_folder:
    database = ycm_core.CompilationDatabase( compilation_database_folder )
else:
    database = None


def DirectoryOfThisScript():
    return os.path.dirname( os.path.abspath( __file__ ) )


def MakeRelativePathsInFlagsAbsolute( flags, working_directory ):
    if not working_directory:
        return list( flags )
    new_flags = []
    make_next_absolute = False
    path_flags = [ '-isystem', '-I', '-iquote', '--sysroot=' ]
    for flag in flags:
        new_flag = flag
        if make_next_absolute:
            make_next_absolute = False
            if not flag.startswith( '/' ):
                new_flag = os.path.join( working_directory, flag )
        for path_flag in path_flags:
            if flag == path_flag:
                make_next_absolute = True
                break
            if flag.startswith( path_flag ):
                path = flag[ len( path_flag ): ]
                new_flag = path_flag + os.path.join( working_directory, path )
                break
        if new_flag:
          new_flags.append( new_flag )
    return new_flags


def FlagsForFile( filename ):
    if database:
        # Bear in mind that compilation_info.compiler_flags_ does NOT return a
        # python list, but a "list-like" StringVec object
        compilation_info = database.GetCompilationInfoForFile( filename )
        final_flags = MakeRelativePathsInFlagsAbsolute(
            compilation_info.compiler_flags_,
            compilation_info.compiler_working_dir_ )
    else:
        relative_to = DirectoryOfThisScript()
        final_flags = MakeRelativePathsInFlagsAbsolute( flags, relative_to )
    return {
        'flags': final_flags,
        'do_cache': True
    }

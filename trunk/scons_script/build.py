import os
import sys

PROJECT_ROOT = '../examples'
if len(sys.argv) != 2:
    print '%s all     -- build all examples' % os.path.basename(sys.argv[0])
    print '%s clean   -- clean all examples' % os.path.basename(sys.argv[0])
    print '%s project -- update all examples\' prject files' % os.path.basename(sys.argv[0])
    sys.exit(0)

# get command options
command = ''
if sys.argv[1] == 'all':
    command = ' '
elif sys.argv[1] == 'clean':
    command = ' -c'
elif sys.argv[1] == 'project':
    command = ' --target=mdk -s'
else:
    print '%s all     -- build all examples' % os.path.basename(sys.argv[0])
    print '%s clean   -- clean all examples' % os.path.basename(sys.argv[0])
    print '%s project -- update all examples\' prject files' % os.path.basename(sys.argv[0])
    sys.exit(0)

projects = os.listdir(PROJECT_ROOT)
for item in projects:
    project_dir = os.path.join(PROJECT_ROOT, item)
    if os.path.isfile(os.path.join(project_dir, 'SConstruct')):
        if os.system('scons --directory=' + project_dir + command) != 0:
            print 'build failed!!'
            break

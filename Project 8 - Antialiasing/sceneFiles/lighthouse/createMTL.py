import os
import sys

# directory name
dirname = 'objFiles'

# extensions
ext = ('.exe', 'obj')

# scanning the directory to get required files
for files in os.scandir(dirname):
    if files.path.endswith(ext):
        fileName= files.name
        print('      <object type="obj" name="sceneFiles/lighthouse/objFiles/' + fileName + '" material="' + fileName +'." />')  # printing file name
#
        # print('    <material type="blinn" name="' + fileName+ '.">')  # printing file name
        # print('      <diffuse texture="sceneFiles/lighthouse/tex/' + fileName + '"/>')
        # print('    </material>')
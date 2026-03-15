import os
import sys
import math
from xml.etree.ElementTree import tostring

# directory name
dirname = 'objFiles'

# extensions
ext = ('.exe', 'obj')
lineRad = 0
print('    <object name = "teapots">')
for j in range(10):
    print('      <object name="line ' + str(j) + '">')
    for i in range(6):
        distance = i/5.0
        radius = 20
        x = 0 + (15 + (i*11))*math.cos(( 2*math.pi) - i/5);
        y = 0 + (15 + (i*11))*math.sin(( 2*math.pi) - i/5);
        print('        <object type="obj" name="sceneFiles/objFiles/teapot.obj" material="whiteMat">')
        print('          <scale value="0.3"/>')
        print('          <translate x="' + str(x) + '" y="' + str(y) + '" z="1"/>')
        print('        </object>')
    lineRad += (36)
    print('        <rotate angle = "' + str(lineRad) + '" z = "1"/>')
    print('      </object>\n')
print('    </object>')
# scanning the directory to get required files
# for files in os.scandir(dirname):
#     if files.path.endswith(ext):
#         fileName= files.name
#         print('      <object type="obj" name="sceneFiles/lighthouse/objFiles/' + fileName + '" material="' + fileName +'." />')  # printing file name
#
        # print('    <material type="blinn" name="' + fileName+ '.">')  # printing file name
        # print('      <diffuse texture="sceneFiles/lighthouse/tex/' + fileName + '"/>')
        # print('    </material>')
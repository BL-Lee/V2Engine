import os
import subprocess

filesToCompile = [
    'trianglevert.vert',
    'trianglefrag.frag',
    'ray.comp',
    'rayProbe.comp',
    'lineVert.vert',
    'lineFrag.frag'
]

compilerPath = 'C:/VulkanSDK/1.3.296.0/Bin/glslc.exe'
shaderPath = '../src/shaders/'

def getShaderString(fileName):
    with open(shaderPath + fileName, 'r') as file:
        lines = file.readlines()
        includeLines = [(lines[i], i) for i in range(len(lines)) if lines[i].startswith("#include")]
        offset = 0
        for (line,i) in includeLines:
            tokens = line.split(" ")
            includedFile = getShaderString(tokens[-1].strip('"\n '))
            lines[i+offset] = ''
            lines = lines[:i + offset] + includedFile + lines[i + offset:]
            offset += len(includedFile)
        return lines
    

for f in filesToCompile:
    glsl = "".join(getShaderString(f))
    tmpFileName = f.split('.')[0] + '_.' + f.split('.')[-1]
    with open(shaderPath + '/processed/' +tmpFileName, 'w') as file:
        file.write(glsl)
    print(tmpFileName)
    subprocess.run([compilerPath, shaderPath + '/processed/' + tmpFileName, '-o' , shaderPath + f.split('.')[0] + '.spv'])

        

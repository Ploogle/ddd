import sys
import getopt

help = 'makemodel.py -f <file> -o <output> -n <name>'

def main(argv):
    filename= ''
    outputpath=''
    name='model'

    # Parse arguments
    try:
        opts, args = getopt.getopt(argv,"hf:n:o",["file=","output=",'name='])
    except getopt.GetoptError:
        print(help)
        sys.exit(2)

    print(opts)

    if (opts == []):
        print(help)
    else:
        for opt, arg in opts:
            if (opt == 'h'):
                print(help)
            elif opt in ("-f", "--file"):
                filename = arg
            elif opt in ("-o", "--output"):
                outputpath = arg
            elif opt in ('-n', "--name"):
                name = arg

    # Handle file
    if (filename):
        vertices = []
        indices = []
        bounds_min = [0, 0, 0]
        bounds_max = [0, 0, 0]
        if (not name):
            name = filename.split(".")[0].split("/")[-1]
        print (name)
        with open(filename, "r") as objfile:
            for line in objfile:
                parts = line.split()
                match parts[0]:
                    case "v": # vertex
                        x = parts[1]
                        y = parts[2]
                        z = parts[3]
                        vertices.append([x, y, z])

                        if (float(x) < bounds_min[0]): bounds_min[0] = float(x)
                        if (float(y) < bounds_min[1]): bounds_min[1] = float(y)
                        if (float(z) < bounds_min[2]): bounds_min[2] = float(z)

                        if (float(x) > bounds_max[0]): bounds_max[0] = float(x)
                        if (float(y) > bounds_max[1]): bounds_max[1] = float(y)
                        if (float(z) > bounds_max[2]): bounds_max[2] = float(z)
                        
                    case "f": # face
                        if len(parts) == 4: # 3 vertices, triangle
                            idx1 = int(parts[1].split('/')[0]) - 1
                            idx2 = int(parts[2].split('/')[0]) - 1
                            idx3 = int(parts[3].split('/')[0]) - 1

                            tri = '\t\t' + ','.join([str(idx1), str(idx2), str(idx3)])
                            indices.append(tri)

                        elif len(parts) == 5: # 4 vertices, face
                            print ("Quad face detected, expected triangle.")
                         

        vertexTemplate = '\t\t{.x = X, .y = Y, .z = Z }'
        verticesOut = []
        for vertex in vertices:
            nv = vertexTemplate
            nv = nv.replace("X", vertex[0])
            nv = nv.replace("Y", vertex[1])
            nv = nv.replace("Z", vertex[2])
            verticesOut.append(nv)

        ifdefName = '__' + name.upper() + '__'

        # Default behavior; replace with actual colors if detected
        # TODO: Detect actual material or vertex colors
        triangleColors = []
        for i in indices:
            triangleColors.append("1")
        output = '#ifndef ' + ifdefName + '\n'\
        '#define ' + ifdefName + '\n' \
        '\n' \
        '#include "../../engine/symbols.h"\n' \
        '\n' \
        'struct Mesh ' + name + ' = {\n'\
        '\t.name="'+name+'",\n'\
        '\t.numVertices = '+str(len(vertices))+',\n'\
        '\t.numIndices = '+str(len(indices) * 3)+',\n'\
        '\t.vertices = (struct Vector3[]) {\n'+\
        ',\n'.join(verticesOut)+'\n'\
        '\t},\n' \
        '\t.min_bounds = {'+','.join(map(str, bounds_min))+"},\n"\
        '\t.max_bounds = {'+','.join(map(str, bounds_max))+"},\n"\
        '\t.indices = (uint16_t[]) {\n'+\
        ',\n'.join(indices)+'\n'\
        '\t},\n' \
        '\t.triangle_colors = (float[]) {\n'+\
        '\t\t'+','.join(triangleColors)+'\n'\
        '\t},\n' \
        '};\n' \
        '\n' \
        '#endif'

        if (outputpath):
            print ('Writing to', outputpath, '...')
            with open(outputpath, 'w') as f:
                print(output, file=f)
        else:
            print ("No output path...")
        print('fin ~')
    

if __name__ == "__main__":
    main(sys.argv[1:])
import sys
import imageio.v3 as iio
import getopt

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

                        # vertexIdx = vertices.index([x, y, z])
                        # indices.append(vertexIdx)

                        # if not [x, y, z] in vertices:
                        #     #print ('added new' + str([x, y, z]))
                        #     vertices.append([x, y, z])
                        #     indices.append(len(vertices) - 2)
                        # else:
                        #     vertexIdx = vertices.index([x, y, z])
                        #     indices.append(vertexIdx)
                            #print ('already in vertices' + str(vertexIdx))
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

        indicesOut = []
        # for i in range(1, len(indices) - 3, 3):
        #     ni = str(indices[i]) + ', ' +\
        #         str(indices[i + 1]) + ', ' +\
        #         str(indices[i + 2])
        #     indicesOut.append('\t\t' + ni)
        
        #print (len(indices))

        output = '#include "../ddd/symbols.h"\n' \
        '\n' \
        'struct Mesh ' + name + ' = {\n'\
        '\t.name="'+name+'",\n'\
        '\t.numVertices = '+str(len(vertices))+',\n'\
        '\t.numIndices = '+str(len(indices) * 3)+',\n'\
        '\t.vertices = (struct Vector3[]) {\n'+\
        ',\n'.join(verticesOut)+'\n'\
        '\t},\n' \
        '\t.indices = (uint16_t[]) {\n'+\
        ',\n'.join(indices)+'\n'\
        '\t}\n' \
        '};'

        if (outputpath):
            print ('Writing to', outputpath, '...')
            with open(outputpath, 'w') as f:
                print(output, file=f)
        else:
            print(output)
        print('fin ~')
    

if __name__ == "__main__":
    main(sys.argv[1:])
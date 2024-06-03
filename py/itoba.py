import sys
import imageio.v3 as iio
import getopt

help = 'itoba.py -f <file>'

def main(argv):
    filename= ''
    outputpath=''
    name='image'

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
        values = []

        print ("Loading", filename, "...")
        image = iio.imread(filename)
        height = len(image)
        for row in image:
            width = len(row)
            for col in row:
                values.append(str(col[0])) # Greyscale, just grab the first channel
                

        output = '#include <stdint.h>\n'\
        '#define '+name.upper()+'_COLUMNS '+str(width)+'\n' \
            '#define '+name.upper()+'_ROWS '+str(height)+'\n'\
            '\n' \
            '#define sample_'+name+'(x, y) ('+name+'_table[y*'+name.upper()+'_COLUMNS+x])\n'\
            '\n' \
            '// '+str(width)+'x'+str(height)+'\n' \
            'uint8_t '+name+'_table['+str(width*height)+'] = {'+\
                (','.join(values))+\
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
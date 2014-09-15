#include "resource_loader.h"
#include <QFile>
#include <iostream>

/**
 * loadTexture: load the texture from a path
 * @param path: the path of the texture
 * @return: the texture handle
 */
GLuint loadTexture(const QString &path)
{

    GLuint result = 0;
    QFile file(path);

    QImage image, texture;

    if(!file.exists())
    {
        std::cerr<<"The path: "<<path.toStdString()
                 <<" does not exist"<<std::endl;
        return result = 0;
    }

    image.load(file.fileName());

    texture = QGLWidget::convertToGLFormat(image);
    texture = texture.mirrored(false, false);
    // We don't use the default alpha
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1,&result);
    glBindTexture(GL_TEXTURE_2D,result);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA, texture.width(), texture.height(),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.bits());
    glBindTexture(GL_TEXTURE_2D, 0);
    return result;
}

void createTexture(GLuint *texName, const int sizeX, const int sizeY)
{

    glGenTextures(1, texName);
    glBindTexture(GL_TEXTURE_2D, *texName);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0,
                 GL_RGBA, sizeX, sizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
}

bool loadBMPTexture(GLuint* texName, const char* filename)
{

    FILE* file;
    unsigned short int bfType;
    long int bfOffBits;
    short int biPlanes;
    short int biBitCount;
    long int biSizeImage;
    unsigned char temp;

    int width, height;
    if((file = fopen(filename, "rb")) == NULL)
    {
        std::cerr<<"Cannot open the file"<<std::endl;
        return false;
    }

    if(!fread(&bfType, sizeof(short int), 1, file))
    {
        std::cerr<<"Error reading file"<<std::endl;
        return false;
    }

    if(bfType != 19778)
    {
        std::cerr<<"This is not a BMP file"<<std::endl;
        return false;
    }

    fseek(file, 8, SEEK_CUR);
    if(!fread(&bfOffBits, sizeof(long int), 1, file))
    {
        std::cerr<<"Error reading file"<<std::endl;
        return false;
    }

    fseek(file, 4, SEEK_CUR);

    fread(&width, sizeof(int), 1, file);
    fread(&height, sizeof(int), 1, file);
    fread(&biPlanes, sizeof(short int), 1, file);

    if(biPlanes != 1)
    {
        std::cerr<<"Number of biplane is not 1"<<std::endl;
        return false;
    }
    // Get the bit per pixel
    fread(&biBitCount, sizeof(short int), 1, file);
    if(biBitCount != 24)
    {
        std::cerr<<" Bits per pixel not 24"<<std::endl;
        return false;
    }

    biSizeImage = width * height * 3;
    char* pixels = (char*)malloc(biSizeImage);

    // Seek the place for storing pixels
    fseek(file, bfOffBits, SEEK_SET);

    if(!fread(pixels, biSizeImage, 1, file))
    {
        std::cerr<<"Error reading file"<<std::endl;
        return false;
    }

    // Swap RGB
    for(int i = 0; i < biSizeImage; i += 3)
    {
        temp = pixels[i];
        pixels[i] = pixels[i + 2];
        pixels[i + 2] = temp;
    }

    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, texName);
    glBindTexture(GL_TEXTURE_2D, *texName);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height,
                 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    free(pixels);
    return true;
}


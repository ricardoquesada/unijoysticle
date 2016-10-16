#include "utils.h"

// Code taken from here:
// https://github.com/anak10thn/graphics-dojo-qt5/blob/master/imagetint/imagetint.cpp
// Convert an image to grayscale and return it as a new image
QImage utils_grayscaled(const QImage &image)
{
    QImage img = image;
    int pixels = img.width() * img.height();
    unsigned int *data = (unsigned int *)img.bits();
    for (int i = 0; i < pixels; ++i) {
        int val = qGray(data[i]);
        data[i] = qRgba(val, val, val, qAlpha(data[i]));
    }
    return img;
}
// Tint an image with the specified color and return it as a new image
QImage utils_tinted(const QImage &image, const QColor &color, QPainter::CompositionMode mode)
{
    QImage resultImage(image.size(), QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&resultImage);
    painter.drawImage(0, 0, utils_grayscaled(image));
    painter.setCompositionMode(mode);
    painter.fillRect(resultImage.rect(), color);
    painter.end();
    resultImage.setAlphaChannel(image.alphaChannel());

    return resultImage;
}

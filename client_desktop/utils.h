#ifndef UTILS_H
#define UTILS_H

#include <QImage>
#include <QPainter>
#include <QColor>

QImage utils_grayscaled(const QImage &image);
QImage utils_tinted(const QImage &image, const QColor &color, QPainter::CompositionMode mode = QPainter::CompositionMode_Screen);

#endif // UTILS_H

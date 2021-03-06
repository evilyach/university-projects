#include "filter.h"

#include <QImage>
#include <QColor>
#include <QPainter>

void Filter::lffBlurApply(QImage *image, filter_kernel_t mode)
{
    const double *kernel = nullptr;
    int w;

    switch (mode) {
        case FK_3x3_K1: {
            static const double k1_3x3[9] = {
                0.1, 0.1, 0.1,
                0.1, 0.2, 0.1,
                0.1, 0.1, 0.1,
            };
            kernel = k1_3x3;
            w = 3;

            break;
        }

        case FK_3x3_K2: {
            static const double k2_3x3[9] = {
                0.0625, 0.125, 0.0625,
                 0.125,  0.25,  0.125,
                0.0625, 0.125, 0.0625
            };
            kernel = k2_3x3;
            w = 3;

            break;
        }

        case FK_3x3_K3: {
            static const double k3_3x3[9] = {
                1/9, 1/9, 1/9,
                1/9, 1/9, 1/9,
                1/9, 1/9, 1/9,
            };
            kernel = k3_3x3;
            w = 3;

            break;
        }

        case FK_5x5_K1: {
            static const double k1_5x5[25] = {
                0.01, 0.02, 0.04, 0.02, 0.01,
                0.02, 0.04, 0.08, 0.04, 0.02,
                0.04, 0.08, 0.16, 0.08, 0.04,
                0.02, 0.04, 0.08, 0.04, 0.02,
                0.01, 0.02, 0.04, 0.02, 0.01
            };
            kernel = k1_5x5;
            w = 5;

            break;
        }

        /*
         * Пример генерации ядра фильтра:
         *
         * ```python
         * import numpy as np
         *
         * a = np.array([
         *   1, 2, 4, 8, 16, 8, 4, 2, 1,
         *   2, 4, 8, 16, 32, 16, 8, 4, 2,
         *   4, 8, 16, 32, 64, 32, 16, 8, 4,
         *   8, 16, 32, 64, 128, 64, 32, 16, 8,
         *   16, 32, 64, 128, 256, 128, 64, 32, 16,
         *   8, 16, 32, 64, 128, 64, 32, 16, 8,
         *   4, 8, 16, 32, 64, 32, 16, 8, 4,
         *   2, 4, 8, 16, 32, 16, 8, 4, 2,
         *   1, 2, 4, 8, 16, 8, 4, 2, 1,
         * ])
         *
         * print(a / sum(a))
         * ```
         * В ответ получаем матрицу, которая подходит условиям задачи.
         */

        case FK_7x7_K1: {
            static const double k1_7x7[49] = {
                0.00206612, 0.00413223, 0.00826446, 0.01652893, 0.00826446, 0.00413223,
                0.00206612, 0.00413223, 0.00826446, 0.01652893, 0.03305785, 0.01652893,
                0.00826446, 0.00413223, 0.00826446, 0.01652893, 0.03305785, 0.0661157,
                0.03305785, 0.01652893, 0.00826446, 0.01652893, 0.03305785, 0.0661157,
                0.1322314,  0.0661157,  0.03305785, 0.01652893, 0.00826446, 0.01652893,
                0.03305785, 0.0661157,  0.03305785, 0.01652893, 0.00826446, 0.00413223,
                0.00826446, 0.01652893, 0.03305785, 0.01652893, 0.00826446, 0.00413223,
                0.00206612, 0.00413223, 0.00826446, 0.01652893, 0.00826446, 0.00413223,
                0.00206612
            };
            kernel = k1_7x7;
            w = 7;

            break;
        }

        case FK_9x9_K1: {
            static const double k1_9x9[81] = {
                0.00047259, 0.00094518, 0.00189036, 0.00378072, 0.00756144, 0.00378072,
                0.00189036, 0.00094518, 0.00047259, 0.00094518, 0.00189036, 0.00378072,
                0.00756144, 0.01512287, 0.00756144, 0.00378072, 0.00189036, 0.00094518,
                0.00189036, 0.00378072, 0.00756144, 0.01512287, 0.03024575, 0.01512287,
                0.00756144, 0.00378072, 0.00189036, 0.00378072, 0.00756144, 0.01512287,
                0.03024575, 0.06049149, 0.03024575, 0.01512287, 0.00756144, 0.00378072,
                0.00756144, 0.01512287, 0.03024575, 0.06049149, 0.12098299, 0.06049149,
                0.03024575, 0.01512287, 0.00756144, 0.00378072, 0.00756144, 0.01512287,
                0.03024575, 0.06049149, 0.03024575, 0.01512287, 0.00756144, 0.00378072,
                0.00189036, 0.00378072, 0.00756144, 0.01512287, 0.03024575, 0.01512287,
                0.00756144, 0.00378072, 0.00189036, 0.00094518, 0.00189036, 0.00378072,
                0.00756144, 0.01512287, 0.00756144, 0.00378072, 0.00189036, 0.00094518,
                0.00047259, 0.00094518, 0.00189036, 0.00378072, 0.00756144, 0.00378072,
                0.00189036, 0.00094518, 0.00047259
            };
            kernel = k1_9x9;
            w = 9;

            break;
        }

        default:
            return;
    }

    // Qt does not allow assigning pixel colors to monochrome images, it throws
    // an exception. That is why always I convert it to Color only to not have
    // a lot of ugly checks.
    image->convertTo(QImage::Format_ARGB32, Qt::ColorOnly);

    QImage *new_image = new QImage(*image);

    for (int x = 0; x < image->width(); x++) {
        for (int y = 0; y < image->height(); y++) {
            double a = 0.0;
            double r = 0.0;
            double g = 0.0;
            double b = 0.0;

            for (int i = 0; i < w; i++) {
                double a2 = 0.0;
                double r2 = 0.0;
                double g2 = 0.0;
                double b2 = 0.0;

                for (int j = 0; j < w; j++) {
                    int x_pos = x - mid(w) + j;
                    int y_pos = y - mid(w) + i;

                    if (x_pos < 0)
                        x_pos = image->width();
                    if (x_pos > image->width())
                        x_pos = 0;
                    if (y_pos < 0)
                        y_pos = image->height();
                    if (y_pos > image->height())
                        y_pos = 0;

                    QColor color = image->pixel(x_pos, y_pos);

                    a2 += color.alpha() * kernel[index(w, i, j)];
                    r2 += color.red()   * kernel[index(w, i, j)];
                    g2 += color.green() * kernel[index(w, i, j)];
                    b2 += color.blue()  * kernel[index(w, i, j)];
                }

                a += a2;
                r += r2;
                g += g2;
                b += b2;
            }

            if (a < 0) a = 0;
            if (a > 255) a = 255;

            if (r < 0) r = 0;
            if (r > 255) r = 255;

            if (g < 0) g = 0;
            if (g > 255) g = 255;

            if (b < 0) b = 0;
            if (b > 255) b = 255;

            new_image->setPixelColor(x, y, qRgba(static_cast<int>(r),
                                                 static_cast<int>(g),
                                                 static_cast<int>(b),
                                                 static_cast<int>(a)));
        }
    }

    QPainter painter(image);
    painter.drawImage(QPoint(0, 0), *new_image);
    painter.end();

    delete new_image;
}


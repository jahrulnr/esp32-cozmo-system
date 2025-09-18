#pragma once
#include "vision/detect/dl_detect_base.hpp"
#include "vision/detect/dl_detect_yolo11_postprocessor.hpp"

namespace coco_detect {
class Yolo11n : public dl::detect::DetectImpl {
public:
    Yolo11n(const char *model_name);
};
} // namespace coco_detect

class COCODetect : public dl::detect::DetectWrapper {
public:
    typedef enum {
        OBJECT_PERSON = 0
    } object_category_t;

    typedef enum {
        YOLO11N_S8_V1,
        YOLO11N_S8_V2,
        YOLO11N_S8_V3,
        YOLO11N_320_S8_V3,
    } model_type_t;
    COCODetect(model_type_t model_type = static_cast<model_type_t>(YOLO11N_320_S8_V3));
};
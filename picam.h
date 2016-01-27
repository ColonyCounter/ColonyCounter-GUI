#ifndef PICAM_H
#define PICAM_H


class PiCam
{
protected:
#if PI_CAM == YES
//Here are all functions that are only defined when compiled for Pi Cam

#endif
public:
    PiCam();
    //Func that gets called but handles the rest
    //StillImage
    //Video
};

#endif // PICAM_H

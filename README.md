# WhistleDetector
Fork of whistle detector from Thomas Hamboeck, Austrian Kangaroos 2014.

## Build
use qibuild

## Calibration for specific whistle
* run whistle_detector in PC
* whistle, and check the ouput
* adjust WhistleBegin and WhistleEnd in WhistleConfig.ini to fit specific whistle
* restart whistle_detector and test until satisfied

# Setup in NAO
* build whistle recognition module with qibuild, copy _WhistleDetector/build-atom/sdk/lib/libwhistle_detector.so_ to _~/lib_ folder in NAO
* copy _WhistleDetector/WhistleConfig.ini_ to _~_ folder in NAO
* add _/home/nao/lib/libwhistle_detector.so_ in _~/naoqi/autoload.ini_

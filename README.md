  This is our R&D Computer Vision project. For now there is no documentation and it's not intended to be used by other companies.
  
  Speed comparison against OpenCV: [speed_compare \(core i7 quad@4ghz\).txt](speed_compare%20(core%20i7%20quad%404ghz).txt)
  
  <hr />
  
  ### Insanely faaaaast ###
  - Hand-written assembler (10% of the code)
  - SIMD (SSE, AVX, NEON)
  - GPGPU (CUDA, OpenVINO, OpenCL, OpenGL, NNAPI and Metal)
  - Smart multithreading (minimal context switch, no false-sharing, no boundaries crossing..)
  - Smart memory access (data alignment, cache pre-load, cache blocking, non-temporal load/store for minimal cache pollution, smart reference counting...)
  - Fixed-point math
  - ... and many more 

### SDKs using our code ###
 - <a target="_blank" href="https://github.com/DoubangoTelecom/KYC-Documents-Verif-SDK">KYC documents recognition & verification </a>
 - <a target="_blank" href="https://github.com/DoubangoTelecom/ultimateFace">Face recognition & 3d liveness detection </a>
 - <a target="_blank" href="https://github.com/DoubangoTelecom/ultimateALPR-SDK">ANPR/ALPR SDK for embedded devices (ARM) and desktops (x86) </a>
 - <a target="_blank" href="https://github.com/DoubangoTelecom/ultimateMRZ-SDK">MRZ/MRP SDK for embedded devices (ARM) and desktops (x86) </a>
 - <a target="_blank" href="https://github.com/DoubangoTelecom/ultimateCreditCard-SDK">Bank credit card recognition/OCR SDK for embedded devices (ARM) and desktops (x86) </a>
 - <a target="_blank" href="https://github.com/DoubangoTelecom/ultimateMRZ-SDK">Bank check information extraction/OCR from Magnetic Ink Character Recognition [MICR] (E-13B & CMC-7) using deep learning for embedded devices (ARM) and desktops (x86) </a>
 
 ### Online demo apps using our code ###
  - <a target="_blank" href="https://doubango.org/webapps/kyc-documents-verif/">KYC documents recognition & verification</a>
 - <a target="_blank" href="https://doubango.org/webapps/face-liveness/">Cloud-based 3D Passive Face Liveness Detection (Anti-Spoofing)</a>
 - <a target="_blank" href="https://doubango.org/webapps/alpr/">Cloud-based Automatic Number/License Plate Recognition (ANPR/ALPR)</a>
 - <a target="_blank" href="https://doubango.org/webapps/mrz/">Cloud-based Machine-readable zone/passport (MRZ/MRP)</a>
 - <a target="_blank" href="https://doubango.org/webapps/credit-card-ocr/">Cloud-based Bank credit card recognition/OCR (ScanToPay)</a>
 - <a target="_blank" href="https://doubango.org/webapps/micr/">Cloud-based Magnetic ink character recognition (MICR E-13B & CMC-7)</a>
 - <a target="_blank" href="https://doubango.org/webapps/cbir/">Cloud-based Content-Based Image Retrieval (CBIR)</a>
 - <a target="_blank" href="https://doubango.org/webapps/ocr/">Cloud-based Scene text recognition (TextInWild)</a>
 
 ### Technical questions ###
 Please check our [discussion group](https://groups.google.com/forum/#!forum/doubango-ai) or [twitter account](https://twitter.com/doubangotelecom?lang=en)

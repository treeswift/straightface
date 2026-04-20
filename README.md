# StraightFace

## Mission
Page photo to page scan: going from loose sheets on the table (skewed, warped, "perspectively transformed", multiple in the same capture) to an OCR PDF document.

The goal is to produce simgle-image-per-page output `ocrmypdf` can readily pick up.

We are aware of `unpaper` and `page-dewarp` but haven't integrated them in our pipeline meaningfully yet.

## State
This is a very early draft but I am under noticeable pressure to make it at least somewhat working in the next few weeks. Stay tuned.

## License
Both copyright and copyleft suck. We don't create algorithms. We discover them. No one should appropriate pure mathematics.

This work, including every single revision of it, is released into the public domain in its entirety and in perpetuity. See the exact license [here](LICENSE).

### Dependencies

We don't redistribute our dependencies (someone else's code you need to acquire in order to build and run our tool) in any form, source or machine-transformed.

Building and running StraightFace is dependent on the presence of OpenCV 4.5 or higher in your system. OpenCV 4.5+ is licensed under a permissive  [Apache 2](https://www.apache.org/licenses/LICENSE-2.0.html) license that allows both commercial and non-commercial use.

# flocking

Sample implementation of Criag Reynolds [flocking boids](http://www.red3d.com/cwr/boids/) algorithm using the olc::PixelGameEngine
from [One Lone Coder](https://community.onelonecoder.com), another great [channel](https://www.youtube.com/channel/UC-yuWVUplUJZvieEligKBkA).

This is very much inspired by the [Nature of Code](https://thecodingtrain.com/learning/nature-of-code/) from The Coding Train which breaks down
many of the techniques here, check it out.

## Cloning

Don't forget `git submodule update --init --recursive` after cloning this repository.

## Usage

Hold `A` + `UP` or `DOWN` Arrow = Increase or Decrease Alignment Weight
Hold `S` + `UP` or `DOWN` Arrow = Increase or Decrease Separation Weight
Hold `C` + `UP` or `DOWN` Arrow = Increase or Decrease Cohesion Weight
`ESC` to Quit
`R` to Reset Flock

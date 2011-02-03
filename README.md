## RSox is a libSoX binding for Ruby

### Usage

    require 'rsox'

	sox = RSox.new
	
	input = sox.open_read 'file.mp3'
	output = sox.open_write 'file.wav', input.signal

	chain = sox.chain input, output
	chain.add 'input', input
	chain.add 'vol', '3dB'
	chain.add 'flanger'
	chain.add 'output', output

	chain.flow

### Thanks

[Roman Golomidov](https://github.com/golomidov) for idea and support

## RSox is a libSoX binding for Ruby

---

### Usage

rewritten `example0.c` from [sox](http://sox.sourceforge.net/)

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

processing samples in Ruby

	require 'rsox'

	sox = RSox.new

	input = sox.open_read 'file.mp3'
	output = sox.open_write 'file.wav', input.signal

	chain = sox.chain input, output
	chain.add 'input', input
	chain.add 'vol', '3dB'
	chain.add 'flanger'
	chain.add 'block' do |buffer|
	  # samples in buffer
	  buffer.size # or buffer.length

      # access samples
	  buffer[0]             # or buffer.at(0)
	  buffer[buffer.size-1] # last sample

      # each sample is a 32bit signed integer converted to Fixnum
	  buffer[0].class == Fixnum
	end

    # all output data will be passed to `block` effect
    # and processed by Ruby code block
	chain.flow

---

### Thanks

[Roman Golomidov](https://github.com/golomidov) for idea and support

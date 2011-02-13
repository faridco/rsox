require 'rsox'

describe 'effect' do
  before :each do
    @rs = RSox.new
  end

  it 'example0.c' do
    input  = @rs.open_read 'input.wav'
    input.signal.channels.should == 2

    output = @rs.open_write 'output.wav', input.signal

    chain = @rs.chain input, output

    chain.add('input', input).should == SOX_SUCCESS
    chain.add('vol', '3dB').should == SOX_SUCCESS
    chain.add('flanger').should == SOX_SUCCESS
    chain.add('output', output).should == SOX_SUCCESS

    chain.flow.should == SOX_SUCCESS
  end

  it 'reverbs' do
    input = @rs.open_read 'input.wav'
    output = @rs.open_write 'output.wav', input.signal

    chain = @rs.chain input, output
    chain.add 'input', input
    chain.add 'reverb'
    chain.add 'output', output

    chain.flow.should == SOX_SUCCESS
  end

  it 'block output' do
    input  = @rs.open_read 'input.wav'
    output = @rs.open_write 'output.wav', input.signal

    puts @rs.buffer_size
    @rs.buffer_size = 16384

    chain = @rs.chain input, output
    chain.add 'input', input
    #chain.add 'reverb'
    chain.add 'block' do |ary|
      #$stderr.puts "#{ary.inspect}: #{ary.length} #{ary.at(0)} #{ary[0]} #{ary[ary.size-1]}"
      #puts ary.at(0)
      #puts ary.size
      #$stderr.puts "ruby: block call |#{ary.inspect}|"
      #File.open('file2.out.mp3.2', 'wb') {|f| f.write ary.pack('l*') }
      
      #`rm -f file3.mp3`
      z = [ary.size]
      (0..ary.size-1).each do |idx|
        z.push ary[idx]
      end
      #File.open('file3.mp3', 'ab') {|f| f.write z.pack('l*') }
    end

    chain.flow.should == SOX_SUCCESS
  end

  it 'check for downrating' do
    input = @rs.open_read 'input.mp3', nil, nil, 'mp3'
    $stderr.puts "input samples: #{input.signal.length}"

    sig = RSoxSignal.new
    sig.bits     = input.signal.bits
    sig.channels = input.signal.channels
    sig.length   = input.signal.length
    sig.rate     = 8000 #input.signal.rate

    $stderr.puts "enc: bps=#{input.encoding.bps}, compression=#{input.encoding.compression}"

    output = @rs.open_write 'output.mp3', input.signal #sig

    chain = @rs.chain input, output
    chain.add 'input', input
    #chain.add 'rate', '8k'

    total_samples = 0
    #chain.add 'block', do |samples|
      #$stderr.puts "block samples: #{samples.length}"
      #total_samples += samples.length
      #(0..5).each{|s| $stderr.puts "sample #{s}: #{samples[s]}" }
    #end
    chain.add 'output', output

    chain.flow.should == SOX_SUCCESS

    $stderr.puts "output samples: #{total_samples}"
  end
end

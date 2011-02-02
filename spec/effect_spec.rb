require 'rsox'

describe 'effect' do
  before :each do
    @rs = RSox.new
  end

  it 'example0.c' do
    input  = @rs.open_read 'file2.mp3'
    input.signal.channels.should == 2

    output = @rs.open_write 'file.out.wav', input.signal

    chain = @rs.chain input, output

    chain.add('input', input).should == SOX_SUCCESS
    chain.add('vol', '3dB').should == SOX_SUCCESS
    chain.add('flanger').should == SOX_SUCCESS
    chain.add('output', output).should == SOX_SUCCESS

    chain.flow.should == SOX_SUCCESS
  end

  it 'reverbs' do
    input = @rs.open_read 'file2.mp3'
    output = @rs.open_write 'file2.out.mp3', input.signal

    chain = @rs.chain input, output
    chain.add 'input', input
    chain.add 'reverb'
    chain.add 'output', output

    chain.flow.should == SOX_SUCCESS
  end

  it 'block output' do
    input  = @rs.open_read 'file2.mp3'
    output = @rs.open_write 'file2.out.mp3', input.signal

    puts @rs.buffer_size
    @rs.buffer_size = 16384

    chain = @rs.chain input, output
    chain.add 'input', input
    #chain.add 'reverb'
    chain.add 'block' do |ary|
      $stderr.puts "#{ary.inspect}: #{ary.length} #{ary.at(0)} #{ary[0]} #{ary[ary.size-1]}"
      #puts ary.at(0)
      #puts ary.size
      #$stderr.puts "ruby: block call |#{ary.inspect}|"
      #File.open('file2.out.mp3.2', 'wb') {|f| f.write ary.pack('l*') }
      
      `rm -f file3.mp3`
      z = [ary.size]
      (0..ary.size-1).each do |idx|
        z.push ary[idx]
      end
      File.open('file3.mp3', 'ab') {|f| f.write z.pack('l*') }
    end

    chain.flow.should == SOX_SUCCESS
  end
end

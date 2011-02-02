require 'rsox'

describe 'effect' do
  before :each do
    @rs = RSox.new
  end

  it 'example0.c' do
    input  = @rs.open_read 'file.mp3'
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

    chain = @rs.chain input, output
    chain.add 'input', input
    chain.add 'reverb'
    chain.add 'block' do |ary|
      #$stderr.puts "ruby: block call |#{ary.inspect}|"
      File.open('file2.out.mp3.2', 'wb') {|f| f.write ary.pack('l*') }
    end

    chain.flow.should == SOX_SUCCESS
  end
end

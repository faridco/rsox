# encoding: utf-8

Gem::Specification.new do |s|
  s.name    = 'rsox'
  s.version = '0.0.1'
  s.date    = Time.new.strftime('%Y-%m-%d')

  s.summary     = 'libSoX binding for Ruby'
  s.description = 'RSox provides simple interface to libSoX, the Swiss Army knife of audio manipulation'

  s.authors  = [ 'Farid Bagishev' ]
  s.email    = 'farid@bagishev.ru'
  s.homepage = 'https://github.com/afhbl/rsox'

  s.rubyforge_project = nil
  s.has_rdoc = false

  s.files = `git ls-files`.split("\n")
  s.extensions = [ "ext/extconf.rb" ]
end

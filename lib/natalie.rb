require_relative 'natalie/compiler'
require_relative 'natalie/compiler2'
require_relative 'natalie/parser'

require_relative 'natalie/repl/legacy'
require_relative 'natalie/repl/main' unless RUBY_ENGINE == 'natalie'

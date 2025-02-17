require_relative './base_instruction'

module Natalie
  class Compiler2
    class DefineBlockInstruction < BaseInstruction
      def initialize(arity:)
        @arity = arity
      end

      def has_body?
        true
      end

      attr_reader :arity

      def to_s
        'define_block'
      end

      def generate(transform)
        body = transform.fetch_block_of_instructions(expected_label: :define_block)
        fn = transform.temp('block')
        transform.with_new_scope(body) do |t|
          body = []
          body << "Value #{fn}(Env *env, Value self, size_t argc, Value *args, Block *block) {"
          body << t.transform('return')
          body << '}'
          transform.top(body)
        end
        transform.push("(new Block(env, self, #{fn}, #{@arity}))")
      end

      def execute(vm)
        start_ip = vm.ip
        vm.skip_block_of_instructions(expected_label: :define_block)
        captured_self = vm.self
        block_proc =
          proc do |*args|
            self_was = vm.self
            vm.self = captured_self
            vm.push_call(return_ip: vm.ip, args: args)
            vm.ip = start_ip
            vm.run
            vm.ip = vm.pop_call[:return_ip]
            vm.self = self_was
            vm.pop # result must be returned from proc
          end
        vm.push(block_proc)
      end
    end
  end
end

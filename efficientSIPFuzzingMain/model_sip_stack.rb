##model_sip_stack.rb

require "./file_analyze"

class ModelSipStack
	def initialize(_sip_stack_name = "", _test_path = "")
		if _sip_stack_name=="" || _test_path==""
			raise ArgumentError, "#{__FILE__}:#{__LINE__}:class ModelSipStack require good input"
		end
		@sip_stack_name = _sip_stack_name
		@test_path = _test_path
		# test_sip_path = test_path+"/parser"
		@test_sip_path = @test_path
		run
	end
	def run
		@test = FileAnalyze.new(@sip_stack_name, @test_path)
		@stack_files = StaticFunction.get_files(@test_sip_path)
		@stack_files.each do |f|
			@test.set_file(f)
			@test.file_cflow_v1
		end
	end
end

sip_stack_name = "opensips_1_10"
test_path = `pwd`.chop! + "/#{sip_stack_name}/parser"
model_sip_stack = ModelSipStack.new(sip_stack_name, test_path)

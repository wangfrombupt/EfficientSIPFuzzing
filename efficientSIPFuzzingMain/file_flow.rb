## => file_flow
class FileFlow
	def initialize
	end
	def flow(_file_name="")
		raise 'using abstract class method: flow'
	end
	def sub_flow
		raise 'using abstract class method: sub_flow'
	end
end

class FileFlowCflow < FileFlow
	def flow(_file_name="")
		@file_inner_res = `cflow #{_file_name} | grep "#{_file_name}"`
	end
	def sub_flow
	end
end

class FileFlowCalltree < FileFlow
	def flow(_file_name="")
		@file_inner_res = `calltree -np -g #{_file_name} | grep "#{_file_name}"`
	end
	def sub_flow
	end
end

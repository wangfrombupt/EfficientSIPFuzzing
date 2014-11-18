##static_function.rb
module StaticFunction
	def self.get_files(_test_path = "")
		@files = []
		if _test_path == ""
			raise ArgumentError, "input error"
		end
		traverse(_test_path)
		#puts @files
		@files
	end
	def self.traverse(_test_path = "")
		if FileTest.directory?(_test_path) then
			dir = Dir.open(_test_path)
			while name = dir.read
				next if name == "."
				next if name == ".."
				traverse(_test_path+"/"+name)
			end
			dir.close
		else
			if /\.c$/  =~ _test_path 
				@files << _test_path
			end
		end
	end
	def self.get_file_headers(_file_name = "")
		# puts _file_name
		fcontent = `cat #{_file_name} | grep include`
		con_array = fcontent.split(/\n/)
		# p con_array
		res = []
		con_array.each do |header|
			if header =~ /include "(.*)"/
				res << $1
			end
		end
		# p res
		res
	end
	def self.get_file_base_path(_file_name = "")
		#p _file_name
		_file_name =~ /(.+\/)(.*c$)/
		$1
	end
	def self.get_file_name(_file_name="")
		_file_name =~ /(.+\/)(.*)\.c$/
		$2
	end
	
end

# test_file = "/home/jinguodong/workspace/sip_stack_analyze_dir/analyze_tool/opensips_1_10/main.c"
# p StaticFunction.get_file_headers(test_file)
# p StaticFunction.get_file_base_path(test_file)


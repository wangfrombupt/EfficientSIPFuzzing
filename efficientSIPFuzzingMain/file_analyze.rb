# file_analyze.rb
#
# StaticFunction should be used as a public module with lot of methods
# => StaticFunction.get_files(path)
# => 	path need to be a path of sip server, like `pwd`.chop!+"/opensips_1_10"
# PublicFunctions should be used as a module that should be include in other module
# => write_to_yaml(file_name, *param)
# =>	yaml param to file_name
# FileAnalyze
# => is a class
# => test = FileAnalyze.new(sip_stack_name, file_name)
# => test.set_file(file_name)
# => test.file_cflow_v1	is for cflow the !#{file_name} which is filtered by /\.c$/ =~ file_name and 
# => yaml the message from the cflow step to #{file_name}.yaml
# => which means each c file response to a c.yaml file

require './mydebug'
require 'yaml'
## => my_dbconnect is required in action_of_mysql.rb
# require './my_dbconnect'
require './action_of_mysql'
require './static_function'
require './public_function'
require './config.rb'
require './file_flow'
require './sip_header_analyze'
# $SIPDB = "opensipstackdb"
# $DBG = "off"
class FileAnalyze
	include MyDebug
	include PublicFunction
	include ActionOfMysql
	attr_reader :path

	def initialize(_sip_stack_name="opensips_1_10", _file_name="")
		# this function makes no sense!!!!!!!!!!!!!!!!!!!!!!
		super
		# if _file_name==""
		# 	raise ArgumentError, "class FileAnalyze require good input"
		# end
		@file_name = _file_name
		@file_inner_res = ""
		@sip_stack_name = _sip_stack_name
		#@path = `pwd`.chop!+"/#{@sip_stack_name}"
		@path = ""
		@drop_table = 1
		@sip_header_analyze = SipHeaderAnalyze.new
		# debug_puts @path
	rescue ArgumentError => error
		puts "#{error.class}, #{error.message}"
	else
		puts "Good, input file:#{@file_name}"
	ensure
		puts "Ensure"
	end

	def set_file(_file_name)
		@file_name = _file_name
	end

	# => insteaded by file_cflow_v1
	def file_cflow
		if @file_name==""
			raise ArgumentError, "#{__FILE__}:#{__LINE__}:class FileAnalyze require good input"
		end
		@file_inner_res = `cflow #{@path}/#{@file_name} | grep "#{@path}\/#{@file_name}"`
		# @file_inner_res = `cflow #{@path}/#{@file_name}`
		@file_functions = @file_inner_res.split(/\n/)
		# debug_puts @file_inner_res
		# debug_puts @file_functions
		#debug_puts @file_functions
		@file_functions = @file_functions.collect{|str|
			str.strip!
			/(^.*)\(\)/ =~ str
			#puts $~
			str = $1
		}
		@file_functions.uniq!
		@file_headers = StaticFunction.get_file_headers(@file_name)
		@file_base_path = StaticFunction.get_file_base_path(@file_name)
		write_to_yaml("#{@file_name}.yaml", 
					{ 	:sip_stack_name => "#{@sip_stack_name}", 
						:file_name => "#{@path}/#{@file_name}", 
						:function_list => "#{@file_functions}", 
						:file_headers => "#{@file_headers}",
						:file_base_path => @file_base_path })
		#write_to_mysql({:sip_stack_name => "#{@sip_stack_name}", :file_name => "#{@path}/#{@file_name}", :function_list => "#{@file_functions}"})
		write_file_to_mysql({ 	:sip_stack_name => "#{@sip_stack_name}", 
								:file_name => "#{@path}/#{@file_name}", 
								:function_list => "#{@file_functions}", 
								:drop_table => @drop_table,
								:file_headers => "#{@file_headers}",
								:file_base_path => @file_base_path })
		@drop_table = 0
		##***********************************

		##***********************************
	end
	def file_cflow_v1
		if @file_name==""
			raise ArgumentError, "#{__FILE__}:#{__LINE__}:class FileAnalyze require good input"
		end
		
		## => FileFlowCalltree use the tool 'calltree'
		puts "doing:#{@path}/#{@file_name}"
		@file_inner_res = FileFlowCalltree.new.flow("#{@path}/#{@file_name}")
		puts "complete:#{@path}/#{@file_name}"

		## => FileFlowCflow use the tool 'cflow'
		# @file_inner_res = FileFlowCflow.new.flow("#{@path}/#{@file_name}")
		
		#puts @file_inner_res
		@file_functions = @file_inner_res.split(/\n/)
		@file_functions = @file_functions.collect{|str|
			str.strip!
			/(^.*) \[/ =~ str
			#puts $~
			str = $1
		}
		@file_functions.uniq!
		#puts @file_functions
		@file_headers = StaticFunction.get_file_headers(@file_name)
		@file_base_path = StaticFunction.get_file_base_path(@file_name)

		write_to_yaml("#{@file_name}.yaml", 
					{ 	:sip_stack_name => "#{@sip_stack_name}", 
						:file_name => "#{@path}#{@file_name}", 
						:function_list => "#{@file_functions}", 
						:file_headers => "#{@file_headers}",
						:file_base_path => @file_base_path })
		#write_to_mysql({:sip_stack_name => "#{@sip_stack_name}", :file_name => "#{@path}/#{@file_name}", :function_list => "#{@file_functions}"})
		
		## => Here, add the process of getting sip headers of the file and the content of the file
		##
		params = {:file_name => "#{@file_name}"}
		sip_header_from_file = @sip_header_analyze.get_sip_header(params)
		params = {:file_name => "#{@file_name}", :content => 1}
		sip_header_from_file_content = @sip_header_analyze.get_sip_header(params)
		# p "sip_header_from_file:#{sip_header_from_file}"
		# p "sip_header_file_content:#{sip_header_file_content}"
		write_file_to_mysql({ 	:sip_stack_name => "#{@sip_stack_name}", 
								:file_name => "#{@path}#{@file_name}", 
								:function_list => "#{@file_functions}", 
								:drop_table => @drop_table,
								:file_headers => "#{@file_headers}",
								:file_base_path => @file_base_path,
								:sip_header_from_file => "#{sip_header_from_file}",
								:sip_header_from_file_content => "#{sip_header_from_file_content}"})

		@drop_table = 0
		file_cflow_pick_subfunctions_v1


		##***********************************

		##***********************************
	end
	def file_cflow_pick_subfunctions
		@functions_res = `cflow #{@path}#{@file_name}`
		@functions_res_list = @functions_res.split(/\n/)
		@functions_hash={}
		# puts @functions_res_array
		@functions_res_list.each do |row|
			flag = 0
			if /^([^ ]*)\(\)/ =~ row
				#puts row
				flag = 1
			else
				if /^[ ]*[^ ]*\(\)/ =~ row
					flag = 2
				end
			end
			puts $~
		end
	end

	def file_cflow_pick_subfunctions_v1
		@functions_res = `calltree -np -bg #{@path}#{@file_name}`
		@functions_res_list = @functions_res.split(/\n/)
		@functions_hash={}
		# puts @functions_res_list
		# puts "" ""
		_i = 0
		_length = @functions_res_list.length
		_flag = "S0"
		_str = ""
		_first_func = ""
		_second_func = ""
		while _i<_length do
			# puts @functions_res_list[_i]
			_str = @functions_res_list[_i]
			debug_puts _str
			if _flag == "S0"
				debug_puts _flag
				if _str =~ /(^[^|]*)[ ]\[/		# remaining
					debug_puts "S0 goto S1"
					_flag = "S1"
					##******do something
					debug_puts $1
					@functions_hash[$1] = Array.new
					_first_func = $1
					##******************
					_i = _i+1
				else
					_i = _i+1
				end
				next
			end
			if _flag == "S1"
				debug_puts _flag
				if _str =~ /(^[^|]+)/
					debug_puts $1
					debug_puts "S1 goto S0"
					_flag = "S0"
				elsif _str =~ /^\|[ ]{3}([^|]+)[ ]\[/
					debug_puts "S1 goto S2"
					_flag = "S2"	
					##******do something
					debug_puts $1
					@functions_hash[_first_func] << $1
					##******************	
					_i = _i+1
				else
					##******do something
					debug_puts _str
					if _str =~ /^\|[ ]{3}([^|]+)/
						@functions_hash[_first_func] << $1
					end
					##******************
					_i = _i+1
				end
				next
			end
			if _flag == "S2"
				debug_puts _flag
				if _str =~ /(^[^|]+)/
					debug_puts $1
					debug_puts "S2 goto S0"
					_flag = "S0"
				elsif _str =~ /^\|[ ]{3}([^|]+)/
					debug_puts $1
					debug_puts "S2 goto S1"
					_flag = "S1"
				else
					_i = _i+1
				end
				next
			end
		end
		@functions_hash.each do |k, v|
			## => Here, add the process of getting sip headers of function
			## 
			params = {:function_name => "#{k}"}
			sip_header_from_function = @sip_header_analyze.get_sip_header(params)
			write_sub_funcs_to_mysql({ 	:sip_stack_name => "#{@sip_stack_name}", 
									:file_name => "#{@path}#{@file_name}", 
									:function_name => "#{k}",
									:sub_functions_list => "#{v}",
									:drop_table => @drop_table,
									:sip_header_from_function => "#{sip_header_from_function}"})
		end
		# @functions_hash
	end

	def init_files_table(_sip_stack_name="", _stack_files)
		## now, this function has no function in fact
		if _sip_stack_name == ""
			raise ArgumentError, "file_analyze init_files_table #{__LINE__} need good input params"
		end
		_stack_files.each do |file_name|
			#puts file_name
			#write_file_to_mysql(_sip_stack_name, file_name)
		end
	end
end

# test = FileAnalyze.new
# sip_stack_name = "opensips_1_10"
# test_path = `pwd`.chop! + "/#{sip_stack_name}/parser/parse_to.c"
# test.set_file(test_path)
# test.file_cflow_v1


# # test on 2013_10_16_14_40 by jinguodong
# sip_stack_name = "opensips_1_10"
# # sip_stack_name = "OpenIMSCore"
# test_path = `pwd`.chop! + "/#{sip_stack_name}"
# test_sip_path = test_path+"/parser"
# test = FileAnalyze.new
# stack_files = StaticFunction.get_files(test_sip_path)
# stack_files.each do |f|
# 	test.set_file(f)
# 	test.file_cflow_v1
# end


# include PublicFunction
# #write_to_yaml("files.yaml", stack_files)
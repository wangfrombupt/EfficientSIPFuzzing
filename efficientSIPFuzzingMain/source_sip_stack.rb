## => source_sip_stack.rb
require './config.rb'
require './mydebug'

module ModelOfSource
	include MyDebug
	require './my_dbconnect'
	def select_from_funcs(_select="", _from="", _where="")
		if _select == "" || _from == "" || _where == ""
			raise ArgumentError, "#{__FILE__}:#{__LINE__}:initialize need right param"
		end
		tquery = DbConnect.new("#{$SIPDB}")
		result = Array.new
		tquery.with_db do |db|
			# p "select #{_select} from #{_from} where #{_where};"
			db.query("select #{_select} from #{_from} where #{_where};") do |res|
				a = {}
				res.each_hash do |k, v|
					a[k] = v
				end
				result.push a
			end
		end
		## => result is a Array, each element means a row gotten in table
		result
		# debug_puts "select #{_select} from #{_from} where #{_where};"
	end
end


class SourceSipStack
	include ModelOfSource
	include MyDebug
	def initialize(_sip_stack_version="", _file_name="", _function_name="", _other_info="")
		super()
		if _sip_stack_version == ""
			raise ArgumentError, "#{__FILE__}:#{__LINE__}:initialize need right param"
		end
		# if _sip_stack_version == "" || _file_name == "" || _function_name == ""
		# 	raise ArgumentError, "#{__FILE__}:#{__LINE__}:initialize need right param"
		# end
		@sip_stack_version = _sip_stack_version
		@file_name = _file_name
		@function_name = _function_name
		@@files_flag_hash = {}
		@@functions_flag_hash = {}
		## => the result of the sourcing process, storing the related file and function.
		@source_result = []
		## => means the result of the sourcing process, like
		## => p @source_sip_result ## => {:to => "true", :from => "true"}
		@source_sip_result = {}
		# if _other_info != ""
		# 	@other_info = _other_info
		# 	debug_puts "other_info #{_other_info}"
		# end
		@other_info = _other_info
	end
	def source_sip_file(_file_name="", _function_name="")
		if _file_name == "" || _function_name == ""
			raise ArgumentError, "#{__FILE__}:#{__LINE__}:initialize need right param"
		end
		## => select * from sip_stack_name_funcs_table where file_name=#{file_name} and function_name=#{function_name}
		result = select_from_funcs("*", "#{@sip_stack_version}_funcs_table", "file_name=\'#{_file_name}\' and function_name=\'#{_function_name}\'")
		# result.each do |ele|
		# 	ele.each do |k|
		# 		debug_puts "#{k}"
		# 	end
		# end
		if result.length == 0
			debug_puts "no row that can match 1 : #{_file_name} : #{_function_name}"
			return ""
		elsif @@files_flag_hash[_file_name] == "true" && @@functions_flag_hash[_function_name] == "true"
			debug_puts "the file had been dealed with : #{_file_name} : #{_function_name}"
			return ""
		else
			# p @@files_flag_hash[_file_name]
			# if @@files_flag_hash[_file_name] == nil
			# 	debug_puts "nil"
			# end
			@@files_flag_hash[_file_name] = "true"
			@@functions_flag_hash[_function_name] = "true"
			# if is_sip_header? _file_name
			# 	debug_puts "get a sip header : #{_file_name}"
			# 	deal_sip_header(_file_name, _function_name, @other_info)
			# end
			deal_sip_header(_file_name, _function_name, @other_info)
			_file_name =~ /(.*)\/(.*).c$/	## => $1 means base_path; $2 means file_name
			single_file_name = $2
			## => get the files that may include this file(_file_name)
			result_1 = select_from_funcs("*", "#{@sip_stack_version}_files_table", "LOCATE(\'#{single_file_name}\', file_headers)>0")
			if result_1.length == 0
				debug_puts "no row that can match 2 : #{_file_name} : #{_function_name}"
				return ""
			else
				result_1.each do |ele|
					table_1 =  ele.keys
					table_1.each do |row_1|
						## => file_name_a means the file that include the file(_file_name)
						file_name_a = row_1["file_name"]
						function_list_a = row_1["function_list"]
						# p "#{file_name_a}:#{function_list_a}"
						if file_name_a == "" || function_list_a == ""
							debug_puts "file_name or function_list be empty : #{_file_name} : #{_function_name}"
							next
						else
							## => get function that call the function(_function_name) and that defined in the file(file_name_a)
							result_2 = select_from_funcs("*", "#{@sip_stack_version}_funcs_table", "file_name=\'#{file_name_a}\' and LOCATE(\'\"#{_function_name}\"\', sub_function_list)>0")
							if result_2.length == 0
								debug_puts "no row that can match 3 : #{_file_name_a} : #{_function_name}"
								next
							else
								result_2.each do |ele_2|
									# p ele_2.keys
									table_2 = ele_2.keys
									table_2.each do |row_2|
										## => function_name_2 is the function in file_name_a that call the function(_function_name)
										function_name_2 = row_2["function_name"]
										debug_puts file_name_a
										debug_puts "#{function_name_2}:#{_function_name}"
										## => file_name_a, function_name_2 are the wanted of the function source_sip_file
										source_sip_file(file_name_a, function_name_2)
									end
									
								end
							end
						end
					end
				end
			end
		end
		@source_result
		@source_sip_result
	end
	def get_source_result
		@source_result
	end
	def get_source_sip_result
		@source_sip_result
	end
	# no used.
	def is_sip_header? (_file_name)
		_file_name =~ /^(.*)\/(.*).c$/
		_single_name = $2
		# p _single_name
		$SIPHD.each do |k, v|
			if _single_name.match(k.to_s)
				# p _single_name.match(k.to_s)
				# debug_puts _single_name
				# @source_sip_result[v] = "true"
				return true
			end
		end
		return false
	end
	def deal_sip_header(_file_name, _function_name, _other_info)
		if _file_name == "" || _function_name == ""
			raise ArgumentError, "#{__FILE__}:#{__LINE__}:initialize need right param"
		end
		@source_result.push({:file_name => "#{_file_name}", :function_name => "#{_function_name}", :other_info => "#{_other_info}"})
	end
	def get_sip_header_from_source_result(type=2)
		# p @source_result
		@source_result.each do |ele|
			if type>0
				get_sip_header_from_source_result_function(ele)
			end
			if type>1
				get_sip_header_from_source_result_file(ele)
			end
			if type>2
				get_sip_header_from_source_result_content(ele)
			end
		end
		get_source_sip_result
	end
	def get_sip_header_from_source_result_function(ele = {})
		file_name = ele[:file_name]
		function_name = ele[:function_name]
		other_info = ele[:other_info]
		result = select_from_funcs("sip_header_from_function", "#{@sip_stack_version}_funcs_table", "file_name=\'#{file_name}\' and function_name=\'#{function_name}\'")
		# p "get sip function #{result}"
		result.each do |ele|
			#p ele.keys
			table = ele.keys
			table.each do |row|
				if row.length==0
					next
				else
					sip_header = Array.new
					_sip_header_str = row["sip_header_from_function"]
					eval "sip_header = #{_sip_header_str}"
					# puts "sip_header = #{_sip_header_str}"
					sip_header.each do |header|
						@source_sip_result[header] = "true"
					end
				end
			end
		end
	end
	def get_sip_header_from_source_result_file(ele = {})
		file_name = ele[:file_name]
		function_name = ele[:function_name]
		other_info = ele[:other_info]
		result = select_from_funcs("sip_header_from_file", "#{@sip_stack_version}_files_table", "file_name=\'#{file_name}\'")
		result.each do |ele|
			# p ele.keys
			table = ele.keys
			table.each do |row|
				if row.length==0
					next
				else
					sip_header = Array.new
					_sip_header_str = row["sip_header_from_file"]
					eval "sip_header = #{_sip_header_str}"
					sip_header.each do |header|
						@source_sip_result[header] = "true"
					end
				end
			end
		end
	end
	def get_sip_header_from_source_result_content(ele = {})
		file_name = ele[:file_name]
		function_name = ele[:function_name]
		other_info = ele[:other_info]
		result = select_from_funcs("sip_header_from_file_content", "#{@sip_stack_version}_files_table", "file_name=\'#{file_name}\'")
		result.each do |ele|
			# p ele.keys
			table = ele.keys
			table.each do |row|
				if row.length==0
					next
				else
					sip_header = Array.new
					_sip_header_str = row["sip_header_from_file_content"]
					eval "sip_header = #{_sip_header_str}"
					sip_header.each do |header|
						@source_sip_result[header] = "true"
					end
				end
			end
		end
	end
	def set_other_info(_other_info = "")
		@other_info = _other_info
	end
	# these variable don't reset after sourcing, 
	def clear
		@@files_flag_hash = {}
		@@functions_flag_hash = {}
		## => the result of the sourcing process, storing the related file and function.
		@source_result = []
		## => means the result of the sourcing process, like
		## => p @source_sip_result ## => {:to => "true", :from => "true"}
		@source_sip_result = {}
	end
end

# sip_stack_version = "opensips_1_10"
# test_path = `pwd`.chop! + "/#{sip_stack_version}"
# test_sip_path = test_path+"/parser"

# # test = SourceSipStack.new(sip_stack_version, "#{test_sip_path}/parse_to.c", "test", "strcpy")
# test = SourceSipStack.new(sip_stack_version)
# test.set_other_info("strcpy")
# test.source_sip_file("#{test_sip_path}/parse_to.c", "parse_to_param")#{test_sip_path}/parse_to.c
# p test.get_source_sip_result
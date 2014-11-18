# coding => utf-8
## => sip_header_analyze.rb
## => Describe
	# test = SipHeaderAnalyze.new
	# test.display	## => show sip headers in the config.rb file.
	## => the main method, get_sip_header(params), params show be a hash.
	## => params = {:file_name => "file_name", :function_name => "function_name", :content => true}
	## => if :file_name exits, get_sip_header from file_name
	## => if :function_name exits and :content be nil or false, get sip header from function name
	## => if :function_name exits and :content not be false or nil, get sip header from content of the file.
	# file_name = "/home/jinguodong/workspace/sip_stack_analyze_dir/analyze_tool/opensips_1_10/parser/parse_content.c"
	# function_name = "parse_content_length"
	# params = {:file_name => "#{file_name}"}
	# p test.get_sip_header(params)	
	# params = {:function_name => "#{function_name}"}
	# p test.get_sip_header(params)
	# params = {:file_name => "#{file_name}", :content => 1}
	# p test.get_sip_header(params)

require './config'
require './static_function'

class SipHeaderAnalyze
	def initialize
		@sip_register = $SIP_REGISTER
	end
	def get_sip_header(_params)
		file_name = _params[:file_name]
		function_name = _params[:function_name]
		content = _params[:content]
		sip_headers = Array.new
		@sip_register.each do |k, v|
			if function_name
				sip_headers << k if is_sip_in_function?(function_name, v[0])
			end
			if file_name && !content
				sip_headers << k if is_sip_in_file?(file_name, v[0])
			end
			if file_name && content
				sip_headers << k if is_sip_in_file_content?(file_name, v[1], k)
			end
		end
		sip_headers
	end
	# check out whether _v in the _function_name, 'i' means IGNORECASE, that is hu lue daxiao xie.
	def is_sip_in_function?(_function_name="", _v="")
		_reg = Regexp.new('_'+_v, 'i')
		if _function_name.match(_reg)
			return true
		end
		return false
	end
	def is_sip_in_file?(_file_name="", _v="")
		_tmp_file_name = StaticFunction.get_file_name(_file_name)
		_reg = Regexp.new('_'+_v, 'i')
		if _tmp_file_name.match(_reg)
			return true
		end
		return false
	end
	def is_sip_in_file_content?(_file_name="", _v="", _k="")
		_reg = Regexp.new(_v)
		# _file_log = `cat #{_file_name}`
		File.open(_file_name, 'r') do |file|
			file.each do |line|
				begin
					if line.match(_reg)
						# puts line
						return true
					end
				rescue StandardError
					# puts "ERROR:#{line}"
				end
			end
		end
		return false
	end

	def show_regexp(_string, reg)
		if _string =~ reg
			p "#{$`}<<#{$&}>>#{$'}"
		else
			puts "no match"
		end
	end
	def display
		puts @sip_register
	end
end

# test = SipHeaderAnalyze.new
# test.display
# file_name = "/home/jinguodong/workspace/sip_stack_analyze_dir/analyze_tool/opensips_1_10/parser/parse_content.c"
# function_name = "parse_content_length"
# params = {:file_name => "#{file_name}"}
# p test.get_sip_header(params)
# params = {:function_name => "#{function_name}"}
# p test.get_sip_header(params)
# params = {:file_name => "#{file_name}", :content => 1}
# p test.get_sip_header(params)
# format_sip_generate.rb
# => 
require './valueProcess.rb'

class FormatSipGenerate
	def initialize
	end
	def get_premessage_from_template(_param_array=nil, _template='')
		if(_param_array == nil || _template == '')
			raise ArgumentError, "#{__FILE__}, #{__LINE__}, params _param_array | _template error"
		end
		deal_template(_param_array, _template)
	end
	# the top level, deal with all setting with param_array
	# param_array:
	# [{'sip_header'=>'header', 'value'=>'value', 'value_class'=>10}, ...]
	# 
	def deal_template(_param_array=nil, _template='')
		template = _template
		_param_array.each do |param|
			sip_header = param['sip_header']
			value = param['value']
			value_class = param['value_class']
			if param['value']['%z'] != nil
				param['value']['%z'] = '%'
			end
			template = deal_template_1(param, template)
		end
		template = deal_template_2(template)
		template
	end
	# the second level, deal with one setting specialed with param
	def deal_template_1(_param=nil, _template='')
		sip_header = _param['sip_header']
		value = _param['value']
		value_class = _param['value_class']
		value_process = ValueProcess.new(value);
		value_res = value_process.transValue
		if _template[/\{\?#{sip_header}\?.*?\}/]!=nil
			_template[/\{\?#{sip_header}\?.*?\}/] = value_res
		end
		# p _template
		_template
	end
	# the second level, deal with the end of the first line
	def deal_template_2(_template='')
		template = _template
		while template[/\{\?.*?\?([\n\r]*.*?)\}/] != nil
			template[/\{\?.*?\?([\n\r]*.*?)\}/]= $1
		end
		template
	end
	#
	def write_to_file(_template='', file_name='')
		begin
			File.open(file_name, 'w') do |file|
				file.write(_template)
			end
		rescue Exception=>err
			puts err.message
		end
	end
end

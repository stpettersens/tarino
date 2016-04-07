{
	'targets': [
		{
			'target_name': 'tarino',
			'include_dirs': [
				"<!(node -e \"require('nan')\")"
			],
			'sources': [ 'native/addon.cc', 'native/tarino.cc' ]
		}
	]
}

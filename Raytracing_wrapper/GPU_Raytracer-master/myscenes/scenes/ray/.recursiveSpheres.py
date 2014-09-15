texture_file = '/course/cs123/data/image/marsTexture.png'

template = '''<scenefile>
	<globaldata>
		<diffusecoeff v="0.7"/>
		<specularcoeff v="0.54"/>
		<ambientcoeff v="0.5"/>
	</globaldata>

	<cameradata>
		<pos x="10" y="4.1" z="16"/>
		<up x="0" y="1" z="0"/>
		
		<heightangle v="49.5"/>
		<look x="-9" y="-3.2" z="-16"/>
	</cameradata>

	<lightdata>
		<id v="0"/>
		<color r="1" g="1" b="1"/>
		<function v1="1.5" v2="0" v3="0"/>
		<position x="10" y="10" z="10"/>
	</lightdata>

	<lightdata>
		<id v="1"/>
		<color r="1" g="1" b="1"/>
		<function v1="1.5" v2="0" v3="0"/>
		<type v="directional"/>
		<direction x="0.25" y="1" z="-1"/>
	</lightdata>

	<lightdata>
		<id v="2"/>
		<color r="1" g="1" b="1"/>
		<function v1="1.5" v2="0" v3="0"/>
		<type v="directional"/>
		<direction x="1" y="-1.8" z="-2"/>
	</lightdata>%s
</scenefile>'''

red = '''
				<reflective r="1" g="0.75" b="0.75"/>
				<diffuse r="1" g="0.75" b="0.75"/>
				<shininess v="25"/>'''

green = '''
				<reflective r="0.75" g="1" b="0.75"/>
				<diffuse r="0.75" g="1" b="0.75"/>
				<shininess v="25"/>'''

blue = '''
				<reflective r="0.75" g="0.75" b="1"/>
				<diffuse r="0.75" g="0.75" b="1"/>
				<shininess v="25"/>'''

def sphere(x, y, z, r, depth):
	color = [red, green, blue][depth % 3]
	return '''
		<transblock>
			<translate x="%s" y="%s" z="%s"/>
			<scale x="%s" y="%s" z="%s"/>
			<object type="primitive" name="sphere">%s
				<specular r="1" g="1" b="1"/>
				<texture file="%s" u="1" v="1"/>
				<blend v="0.75"/>
			</object>
		</transblock>''' % tuple(map(str, [x, y, z, r, r, r, color, texture_file]))

def transblock(x, y, z, axis_x, axis_y, axis_z, angle, name):
	return '''
		<transblock>
			<translate x="%s" y="%s" z="%s"/>
			<rotate x="%s" y="%s" z="%s" angle="%s"/>
			<object type="master" name="%s" />
		</transblock>''' % tuple(map(str, [x, y, z, axis_x, axis_y, axis_z, angle, name]))

def recursion(x, y, z, r, depth):
	name = 'level%d' % depth if depth else 'root'
	extra = ''
	xml = ''

	if depth < MAX_DEPTH:
		s = r * 0.75
		next_level = 'level%d' % (depth + 1)
		xml += recursion(0, 0, 0, r * 0.5, depth + 1)
		extra += transblock(x - s, y, z, 0, 0, 1, 90, next_level)
		extra += transblock(x + s, y, z, 0, 0, 1, -90, next_level)
		extra += transblock(x, y + s, z, 1, 0, 0, 0, next_level)
		extra += transblock(x, y - s, z, 0, 0, 1, 180, next_level) if depth == 0 else ''
		extra += transblock(x, y, z - s, 1, 0, 0, -90, next_level)
		extra += transblock(x, y, z + s, 1, 0, 0, 90, next_level)

	return xml + '''\n
	<object type="tree" name="%s">%s
	</object>''' % (name, sphere(x, y, z, r, depth) + extra)

for MAX_DEPTH in range(10):
	xml = template % recursion(0, 0, 0, 6, 0)
	f = open('recursiveSpheres%d.xml' % (MAX_DEPTH + 1), 'w')
	f.write(xml)
	f.close()

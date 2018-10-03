'''
Just to test that build is successful and built files are in place.
'''

from pathlib import Path

artifacts = (Path(__file__).parent / '../artifacts').absolute()
assert (artifacts / 'lib/Reaktoro.lib').is_file()
assert (artifacts / 'bin/Reaktoro.dll').is_file()
assert (artifacts / 'include/Reaktoro.hpp').is_file()

import reaktoro
assert hasattr(reaktoro, 'Reaction')

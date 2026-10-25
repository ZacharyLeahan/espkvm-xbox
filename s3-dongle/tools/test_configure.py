# SPDX-License-Identifier: Apache-2.0
import contextlib
import io
import json
from pathlib import Path
import stat
import sys
import tempfile
import unittest
from unittest.mock import patch
import configure

class ConfigureTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup)
        self.root=Path(self.temp.name)
        (self.root/'.env').write_text('WIFI_SSID=TestNetwork\nWIFI_PASSWORD=test-only-password\n')
        self.patch=patch.object(configure,'ROOT',self.root);self.patch.start();self.addCleanup(self.patch.stop)
    def run_configure(self,*args):
        output=io.StringIO()
        with patch.object(sys,'argv',['configure.py',*args]),contextlib.redirect_stdout(output):configure.main()
        return output.getvalue()
    def test_pairing_is_private_and_reused(self):
        output=self.run_configure('--mac','ac:27:6e:b0:ce:a4')
        pair=json.loads((self.root/'.env.s3-route').read_text())
        self.assertEqual(len(pair['key']),64)
        self.assertNotIn(pair['key'],output);self.assertNotIn('test-only-password',output)
        self.run_configure();self.assertEqual(pair,json.loads((self.root/'.env.s3-route').read_text()))
        for path in [self.root/'.env.s3-route',*self.root.glob('boards/*')]:self.assertEqual(stat.S_IMODE(path.stat().st_mode),0o600)
    def test_existing_profiles_are_preserved(self):
        p4=self.root/'build.profile/sdkconfig';p4.parent.mkdir();p4.write_text('CONFIG_KEEP_AUDIO=y\n# CONFIG_KVM_XBOX_S3_PLAYER1 is not set\n')
        s3=self.root/'s3-dongle/sdkconfig';s3.parent.mkdir();s3.write_text('CONFIG_FREERTOS_HZ=100\n')
        self.run_configure('--mac','ac:27:6e:b0:ce:a4','--main-config',str(p4))
        self.assertIn('CONFIG_KEEP_AUDIO=y',p4.read_text());self.assertIn('CONFIG_KVM_XBOX_S3_PLAYER1=y',p4.read_text())
        self.assertIn('CONFIG_FREERTOS_HZ=100',s3.read_text())
    def test_four_players_preserve_key_and_existing_identity(self):
        self.run_configure('--mac','ac:27:6e:b0:ce:a4')
        first=json.loads((self.root/'.env.s3-route').read_text())
        for player in (2,3,4):
            self.run_configure('--player',str(player),'--mac',f'02:00:00:00:00:0{player}')
        pair=json.loads((self.root/'.env.s3-route').read_text())
        self.assertEqual(pair['key'],first['key']);self.assertEqual(pair['mac'],first['mac'])
        self.assertEqual(len(pair['players']),4)
        with self.assertRaises(ValueError):self.run_configure('--player','2','--mac',first['mac'])
        self.assertEqual(pair,json.loads((self.root/'.env.s3-route').read_text()))
    def test_migrates_original_pairing(self):
        old={'key':'ab'*32,'mac':'ac:27:6e:b0:ce:a4'}
        (self.root/'.env.s3-route').write_text(json.dumps(old))
        self.run_configure('--player','2','--mac','02:00:00:00:00:02')
        pair=json.loads((self.root/'.env.s3-route').read_text())
        self.assertEqual(pair['key'],old['key']);self.assertEqual(pair['players']['1'],old['mac'])

    def test_refuses_invalid_identity_and_symlink(self):
        with self.assertRaises(ValueError):self.run_configure('--mac','not-a-mac')
        target=self.root/'target';target.write_text('do not overwrite')
        (self.root/'.env.s3-route').symlink_to(target)
        with self.assertRaises(ValueError):configure.private_write(self.root/'.env.s3-route','replacement')
        self.assertEqual(target.read_text(),'do not overwrite')

if __name__=='__main__':unittest.main()

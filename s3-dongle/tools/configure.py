#!/usr/bin/env python3
# SPDX-License-Identifier: Apache-2.0
"""Create ignored multi-player build settings without printing home credentials."""
import argparse
import json
import os
from pathlib import Path
import secrets
import sys

ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'tools'))
from build_xbox import parse_env

def private_write(path,text):
    if path.is_symlink():raise ValueError('Refusing a configuration symlink')
    path.parent.mkdir(parents=True,exist_ok=True)
    fd=os.open(path,os.O_WRONLY|os.O_CREAT|os.O_TRUNC,0o600)
    with os.fdopen(fd,'w') as stream:
        os.fchmod(stream.fileno(),0o600);stream.write(text)

def update(text,settings):
    lines=[line for line in text.splitlines() if not any(line.startswith(k+'=') or line==f'# {k} is not set' for k in settings)]
    return '\n'.join(lines)+'\n'+''.join(k+'='+v+'\n' for k,v in settings.items())

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--mac',help='S3 station MAC for the selected player')
    parser.add_argument('--player',type=int,choices=range(1,5),default=1)
    parser.add_argument('--main-config',type=Path,help='Existing P4 sdkconfig to preserve; otherwise only generate a private overlay')
    args=parser.parse_args()
    config=None
    if args.main_config:
        if args.main_config.is_symlink():raise ValueError('Refusing a configuration symlink')
        config=args.main_config.resolve()
        if not config.is_relative_to(ROOT.resolve()) or config.name!='sdkconfig':
            raise ValueError('Main sdkconfig must be named sdkconfig inside this checkout')
    wifi=parse_env((ROOT/'.env').read_text())
    if not wifi['WIFI_SSID']:raise ValueError('Set WIFI_SSID in the ignored root .env first')
    pair_path=ROOT/'.env.s3-route'
    pair=json.loads(pair_path.read_text()) if pair_path.exists() else {'key':secrets.token_hex(32)}
    players=pair.get('players',{'1':pair['mac']} if pair.get('mac') else {})
    mac=args.mac or players.get(str(args.player),'')
    if len(mac.split(':'))!=6 or any(len(b)!=2 or any(c not in '0123456789abcdefABCDEF' for c in b) for b in mac.split(':')):
        raise ValueError('Supply the identified S3 station MAC with --mac')
    if len(pair.get('key',''))!=64 or any(c not in '0123456789abcdef' for c in pair['key']):raise ValueError('Invalid saved pairing key')
    mac=mac.lower()
    if any(value==mac and slot!=str(args.player) for slot,value in players.items()):
        raise ValueError('This board is already assigned to another player')
    players[str(args.player)]=mac
    if '1' not in players:raise ValueError('Configure player 1 before additional players')
    pair['players']=players
    pair['mac']=players['1'];private_write(pair_path,json.dumps(pair)+'\n')
    common={'CONFIG_KVM_XBOX_S3_KEY':json.dumps(pair['key'])}
    s3={**common,'CONFIG_KVM_WIFI_DEFAULT_SSID':json.dumps(wifi['WIFI_SSID']),'CONFIG_KVM_WIFI_DEFAULT_PASSWORD':json.dumps(wifi['WIFI_PASSWORD'])}
    main_settings={**common,'CONFIG_KVM_USB_XBOX':'y','CONFIG_KVM_XBOX_S3_PLAYER1':'y','CONFIG_KVM_XBOX_S3_MAC':json.dumps(pair['mac'])}
    main_settings.update({f'CONFIG_KVM_XBOX_S3_MAC{n}':json.dumps(players.get(str(n),'')) for n in (2,3,4)})
    private_write(ROOT/'boards/s3_player1.private.defaults',update('# Generated locally; contains credentials.\n',s3))
    private_write(ROOT/'boards/s3_route.private.defaults',update('# Generated locally; contains pairing material.\n',main_settings))
    s3_config=ROOT/'s3-dongle/sdkconfig'
    if s3_config.exists():private_write(s3_config,update(s3_config.read_text(),s3))
    if config:private_write(config,update(config.read_text(),main_settings))
    print('Generated ignored private overlays; saved pairing key preserved. No credentials printed.')

if __name__=='__main__':
    try:main()
    except (ValueError,OSError,json.JSONDecodeError) as error:
        print(f'Configuration failed: {error}',file=sys.stderr);sys.exit(1)

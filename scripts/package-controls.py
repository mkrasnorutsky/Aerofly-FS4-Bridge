"""Package the locally built DLL, protocol reference and required notices."""
from pathlib import Path
import subprocess
import shutil
import json
import hashlib
import zipfile

root=Path(__file__).resolve().parent.parent
manifest=subprocess.check_output([str(root/'build/Release/CommandContractTests.exe'),'--manifest'])
(root/'reference/command_catalog.json').write_text(json.dumps(json.loads(manifest),indent=2)+'\n')
package=root/'dist/AeroflyBridge-0.4.1-x64'
package.mkdir(parents=True,exist_ok=True)
for source,target in [('build/Release/AeroflyBridge.dll','AeroflyBridge.dll'),
                      ('docs/command_support.md','COMMANDS.md'),('LICENSE','LICENSE.txt'),
                      ('reference/command_catalog.json','command_catalog.json')]:
    shutil.copy2(root/source,package/target)
notices=[]
for name,file in [('IPACS SDK','tm_external_message.h'),
                  ('nlohmann/json','build/_deps/nlohmann_json-src/LICENSE.MIT'),
                  ('spdlog','build/_deps/spdlog-src/LICENSE')]:
    text=(root/file).read_text(encoding='utf-8-sig')
    if name=='IPACS SDK': text=text.split('#ifndef')[0]
    notices.append(name+'\n'+text)
(package/'THIRD-PARTY-NOTICES.txt').write_text('\n\n'.join(notices),encoding='utf-8')
(package/'README.txt').write_text('''Aerofly Bridge 0.4.1 (Windows x64)

Close Aerofly FS4 and the Aerofly Bridge Tester before replacing a DLL.
Back up the existing AeroflyBridge.dll, then copy this DLL to your actual
Documents/Aerofly FS 4/external_dll folder (Documents may be in OneDrive).
Keep the filename AeroflyBridge.dll. Start Aerofly and load a flight.
For testing without Aerofly, choose this DLL in the Bridge Tester instead.
Do not run Aerofly and the tester together.

This build has passed real-DLL command transport tests for all 197 writable
SDK variants over TCP and WebSocket. Aircraft response is not yet verified
in Aerofly. Some controls are aircraft-specific. See COMMANDS.md.
The iPhone app needs to send the newly supported commands to use them.

Source: https://github.com/mkrasnorutsky/Aerofly-FS4-Bridge
Build: MSVC 2026 x64 Release; Microsoft Visual C++ x64 runtime required.
''')
digest=hashlib.sha256((package/'AeroflyBridge.dll').read_bytes()).hexdigest()
(package/'SHA256.txt').write_text(digest+'  AeroflyBridge.dll\n')
archive=root/'dist/AeroflyBridge-0.4.1-x64.zip'
with zipfile.ZipFile(archive,'w',zipfile.ZIP_DEFLATED) as zipped:
    for file in package.iterdir(): zipped.write(file,file.name)
print('Package:',archive)
print('DLL SHA256:',digest)

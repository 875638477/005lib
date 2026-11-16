from pathlib import Path
import sys,json,copy,tempfile,unittest,hashlib
sys.path.insert(0,str(Path(__file__).resolve().parent))
from check_rc_layout_contract import audit,sha
class Rules(unittest.TestCase):
 def setUp(self):
  self.tmp=tempfile.TemporaryDirectory();self.root=Path(self.tmp.name);xml=self.root/'source.xml';xml.write_text('<fixture/>');lib=self.root/'01captureLib/000Resistor.OLB';lib.parent.mkdir();lib.write_bytes(b'synthetic library fixture, not a Cadence OLB')
  proof=self.root/'bound_proof.json';proof.write_text(json.dumps(dict(source_xml_sha256=sha(xml),fixture_only=True)));ev=dict(path=str(proof),sha256=sha(proof))
  self.f=dict(rule_version='RC-20261116.2',source_xml=dict(path=str(xml),sha256=sha(xml)),inventory=['P/R1'],parts=[dict(id='P/R1',symbol_type='R',component_axis='vertical',texts=dict(ref=dict(rotation_quadrants=1,anchor=[0,0]),value=dict(rotation_quadrants=1,anchor=[0,20])),library=dict(path=str(lib),package_name='R',file_sha256=sha(lib),cache_shape_sha256='approved-shape',public_shape_sha256='approved-shape'),alignment=dict(bound_native_reference=True,evidence=ev,ref_baseline_line=[[0,-20],[0,0]],value_baseline_line=[[0,20],[0,40]],allowed_native_rounding_doc=0),grid_doc=5,electrical_grid_points=[[0,0],[0,20]],electrical_grid_coverage_complete=True,clearance=dict(bound_full_scene=True,evidence=ev,covered_roles=['ref','value'],unresolved_objects=0,interferences=[]))])
 def tearDown(self):self.tmp.cleanup()
 def result(self):return audit(self.f,self.root)
 def test_correct_vertical_fixture(self):self.assertEqual(self.result()['status'],'PASS_RC_FACTS')
 def test_value_only_rotated_is_rejected(self):
  self.f['parts'][0]['texts']['ref']['rotation_quadrants']=0;self.assertEqual(self.result()['parts'][0]['checks']['RCAXIS'],'FAIL')
 def test_on_grid_but_off_lead_line_is_rejected(self):
  self.f['parts'][0]['texts']['ref']['anchor']=[5,0];r=self.result()['parts'][0]['checks'];self.assertEqual(r['RCGRID'],'PASS');self.assertEqual(r['RCALIGN'],'FAIL')
 def test_named_local_library_with_changed_cache_rejected(self):
  self.f['parts'][0]['library']['cache_shape_sha256']='scaled-copy';self.assertEqual(self.result()['parts'][0]['checks']['RCLIB'],'FAIL')
 def test_missing_instance_blocks_pass(self):
  self.f['inventory'].append('P/C2');self.assertFalse(self.result()['coverage_complete'])
 def test_missing_scene_is_unknown(self):
  self.f['parts'][0]['clearance']={};self.assertEqual(self.result()['parts'][0]['checks']['RCCLEAR'],'UNKNOWN');self.assertNotEqual(self.result()['status'],'PASS_RC_FACTS')
 def test_reference_checkbox_without_bound_file_is_unknown(self):
  del self.f['parts'][0]['alignment']['evidence'];self.assertEqual(self.result()['parts'][0]['checks']['RCALIGN'],'UNKNOWN')
 def test_stale_source_rejected(self):
  Path(self.f['source_xml']['path']).write_text('<changed/>')
  with self.assertRaises(AssertionError):self.result()
 def test_empty_inventory_not_pass(self):
  self.f['inventory']=[];self.f['parts']=[];self.assertNotEqual(self.result()['status'],'PASS_RC_FACTS')
 def test_real_interference_blocks_pass(self):
  self.f['parts'][0]['clearance']['interferences']=['Ref intersects wire'];self.assertEqual(self.result()['parts'][0]['checks']['RCCLEAR'],'FAIL')
 def test_other_local_library_is_rejected(self):
  p=self.f['parts'][0];f=self.root/'01captureLib/COMMON_PC.OLB';f.write_bytes(b'another local library');p['library']['path']=str(f);p['library']['file_sha256']=sha(f);self.assertEqual(self.result()['parts'][0]['checks']['RCLIB'],'FAIL')
 def test_wrong_package_in_selected_library_rejected(self):
  self.f['parts'][0]['library']['package_name']='CORE_R';self.assertEqual(self.result()['parts'][0]['checks']['RCLIB'],'FAIL')
 def test_polar_cap_requires_selected_polar_symbol(self):
  p=self.f['parts'][0];p['symbol_type']='C_ELEC';self.assertEqual(self.result()['parts'][0]['checks']['RCLIB'],'FAIL')
if __name__=='__main__':unittest.main(verbosity=2)

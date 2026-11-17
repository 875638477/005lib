"""Synthetic regression of proximity gating; never certifies a CAD design."""
import hashlib,json,os,pathlib,sys,tempfile,unittest
sys.path.insert(0,str(pathlib.Path(__file__).resolve().parents[1]))
from check_rc_layout_contract import proximity_result,check_part

class CaptionProximity(unittest.TestCase):
    def setUp(self):
        self.tmp=tempfile.TemporaryDirectory(dir=os.environ.get('CAPTURE_TEST_TMP_ROOT'))
        self.source='synthetic-rule-regression-not-cad'
        p=pathlib.Path(self.tmp.name)/'proof.json';p.write_text(json.dumps({'source_xml_sha256':self.source}))
        self.evidence={'path':str(p),'sha256':hashlib.sha256(p.read_bytes()).hexdigest()}
    def tearDown(self):self.tmp.cleanup()
    def profile(self,gap):
        caption={'full_ink_bounds':[gap,10,gap+6,30],'own_lead_line':[[0,0],[0,40]],'approved_horizontal_ink_gap_doc':2}
        return {'proximity':{'native_text_frame_bound':True,'approved_reference_id':'synthetic-horizontal-positive','evidence':self.evidence,'allowed_quantization_doc':5,'ref':dict(caption),'value':dict(caption)}}
    def test_far_vertical_ink_is_rejected(self):
        status,detail=proximity_result(self.profile(10),'vertical',5,self.source)
        self.assertEqual(status,'FAIL');self.assertEqual(detail['ref_perpendicular_ink_gap_doc'],10)
    def test_close_vertical_ink_can_pass_geometry_gate(self):
        self.assertEqual(proximity_result(self.profile(3),'vertical',5,self.source)[0],'PASS')
    def test_missing_native_text_frame_is_unknown(self):
        p=self.profile(3);p['proximity']['native_text_frame_bound']=False
        self.assertEqual(proximity_result(p,'vertical',5,self.source)[0],'UNKNOWN')
    def test_old_anchor_only_proof_cannot_pass_alignment(self):
        p={'id':'synthetic','component_axis':'vertical','texts':{'ref':{'rotation_quadrants':3,'anchor':[0,0]},'value':{'rotation_quadrants':3,'anchor':[0,0]}},'alignment':{'bound_native_reference':True,'evidence':self.evidence,'allowed_native_rounding_doc':0,'ref_baseline_line':[[0,0],[0,10]],'value_baseline_line':[[0,0],[0,10]]},'grid_doc':5}
        self.assertEqual(check_part(p,pathlib.Path(self.tmp.name),self.source,{})['checks']['RCALIGN'],'UNKNOWN')

if __name__=='__main__':unittest.main()

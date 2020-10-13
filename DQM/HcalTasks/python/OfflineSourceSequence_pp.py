import FWCore.ParameterSet.Config as cms

#-----------------
#	HCAL DQM Offline Source Sequence Definition for pp
#	To be used for Offline DQM importing
#-----------------

#	import the tasks
from DQM.HcalTasks.DigiTask import digiTask
from DQM.HcalTasks.RawTask import rawTask
from DQM.HcalTasks.TPTask import tpTask
from DQM.HcalTasks.RecHitTask import recHitTask

#	set processing type to Offine
digiTask.ptype = cms.untracked.int32(1)
tpTask.ptype = cms.untracked.int32(1)
recHitTask.ptype = cms.untracked.int32(1)
rawTask.ptype = cms.untracked.int32(1)

#	set the label for Emulator TP Task
tpTask.tagEmul = cms.untracked.InputTag("valHcalTriggerPrimitiveDigis")

hcalOfflineSourceSequence = cms.Sequence(
	digiTask
	+tpTask
	+recHitTask
	+rawTask)


from Configuration.Eras.Modifier_phase2_hcal_cff import phase2_hcal
_phase2_hcalOfflineSourceSequence = hcalOfflineSourceSequence.copyAndExclude([tpTask,rawTask])
phase2_hcal.toReplaceWith(hcalOfflineSourceSequence, _phase2_hcalOfflineSourceSequence)
phase2_hcal.toModify(digiTask,
    tagHBHE = cms.untracked.InputTag("simHcalDigis","HBHEQIE11DigiCollection"),
    tagHO = cms.untracked.InputTag("simHcalDigis"),
    tagHF = cms.untracked.InputTag("simHcalDigis","HFQIE10DigiCollection")
)

from Configuration.ProcessModifiers.premix_stage2_cff import premix_stage2
(premix_stage2 & phase2_hcal).toModify(digiTask,
    tagHBHE = "DMHcalDigis:HBHEQIE11DigiCollection",
    tagHO = "DMHcalDigis",
    tagHF = "DMHcalDigis:HFQIE10DigiCollection"
)

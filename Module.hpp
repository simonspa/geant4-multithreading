#pragma once

#include <G4Types.hh>

class G4Run;


class Module {
    public:
        Module();
//unsigned int GetSlotId();  //
        // Init to be called from main thread
        void init();

        bool run(int evt_nr);

        void finialize();
    private:
       // G4int m_nEvents;
        //G4int m_taskID;
        G4bool m_beamOnCondition;
};
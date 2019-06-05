#pragma once

class SimpleMasterRunManager;

class Module {
    public:
        Module(SimpleMasterRunManager* runmanager);
        void init();

        bool run(int evt_nr);

        void finialize();

        // must be called by each thread to cleanup thread local data
        void finializeForThread();

    private:
        // The new G4RunManager
        SimpleMasterRunManager* run_manager_;
};
#pragma once

class SimpleMasterRunManager;
class Module {
    public:
        Module(SimpleMasterRunManager* runmanager);
        void init();

        bool run(int evt_nr);

        void finialize();

        void cleanup();
    private:
      SimpleMasterRunManager* run_manager_;
};
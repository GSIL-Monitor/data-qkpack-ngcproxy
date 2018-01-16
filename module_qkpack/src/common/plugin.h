/*
 * Author: lihang <lihang01@youku.com>
 *
 * Create Date: 2015-08
 *
 */

#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include <string>

class CPlugin {
    public:
        CPlugin() {}
		virtual ~CPlugin() {}


    public:
        // init plugin in nginx master process
        virtual int InitMaster(const std::string &so_conf) {
            (void)so_conf;
            return 0;
        }

        // init plugin in nginx worker process
        virtual int InitProcess(const std::string &so_conf) {
            (void)so_conf;
            return 0;
        }
	
		//timer one min publish
		virtual int Publish() {
			return 0;
		}


		/* callback of upstream or subrequest */
        virtual int CreateRequest(void *data) { 
            (void)data;
            return 0; 
        }
		
		/* callback of upstream or subrequest */
        virtual int ProcessBody(void *data) {
            (void)data;
            return 0;
        }

		/* callback of upstream*/
		virtual int ReinitRequest(void *data) {
			(void)data;
			return 0;
		}

        virtual int Destroy(void *data) {
            (void)data;
			return 0;
        }

        virtual void ExitProcess() {}
        virtual void ExitMaster() {}
};

#endif 

//TODO: implementar o throw para cada código de erro
#include "ptgrey.hpp"

namespace is {
namespace cameras {

using namespace FlyCapture2;
using namespace std;

PointGrey::PointGrey(PGRGuid* handle) {
	this->handle = handle;
	this->running = false;
	this->first_run = true;
	this->query_max_fps();
	this->query_max_trigger_delay();
	this->query_max_resolution();
	this->query_serial_number();
}

PointGrey::~PointGrey() {
	this->disconnect();
	if(this->handle != nullptr)
		delete this->handle;
}

bool PointGrey::connect(){
	if(!this->camera.IsConnected()) {
		Error error;
		error = this->camera.Connect(this->handle);
		if (error != PGRERROR_OK) {
			error.PrintErrorTrace();
			return false;
		}
		return true;
	}
	return false;
}

bool PointGrey::disconnect(){
	if(this->camera.IsConnected()) {
		Error error;
		error = this->camera.Disconnect();
		if (error != PGRERROR_OK) {
			error.PrintErrorTrace();
			return false;
		}
		return true;
	}
	return true;
}

vector<unique_ptr<PointGrey>> PointGrey::discovery() {
	vector<unique_ptr<PointGrey>> cameras;
	BusManager bus;
	unsigned int numCameras;
	Error error;
	error = bus.GetNumOfCameras(&numCameras);
	if (error != PGRERROR_OK) {
		error.PrintErrorTrace();
	}
	for (unsigned int i=0; i < numCameras; i++) {
		PGRGuid* guid = new PGRGuid();
		error = bus.GetCameraFromIndex(i, guid);
		if (error != PGRERROR_OK) {
			error.PrintErrorTrace();
		} else {
			cameras.emplace_back(new PointGrey(guid));
		}
	}
	return cameras;
}
//TODO: implementar throw
unique_ptr<PointGrey> PointGrey::discovery(const std::string& ip_address) {
	BusManager bus;
	Error error;
	int ip_int = is::utils::ip_to_int(ip_address.c_str());
	if (!ip_int) {
		//invalid ip
		cout << "Invalid IP address." << endl;
		return nullptr;
	}
	IPAddress ip(ip_int);
	PGRGuid* guid = new PGRGuid();
	error = bus.GetCameraFromIPAddress(ip, guid);
	if (error != PGRERROR_OK) {
		error.PrintErrorTrace();
		return nullptr;
	}
	unique_ptr<PointGrey> camera(new PointGrey(guid));
	return camera;
}

void PointGrey::on_image_grabbed(FlyCapture2::Image* pImage, const void* pCallbackData) {
  	PointGrey* self = (PointGrey*)pCallbackData;
	// Update timestamp
	auto ts = std::chrono::system_clock::now().time_since_epoch().count();
	// Convert image
  	FlyCapture2::Image converted;

  	int cvtype;
  	switch(self->get_image_type()) {
	case ImageType::rgb:
		pImage->Convert(FlyCapture2::PIXEL_FORMAT_BGR, &converted);
		cvtype = CV_8UC3;
		break;
	case ImageType::grayscale:
    default:
		cvtype = CV_8UC1;
    	pImage->Convert(FlyCapture2::PIXEL_FORMAT_MONO8, &converted);
		break;
	}
    
    Image x (converted.GetData(), converted.GetDataSize(),
		   	   converted.GetRows(), converted.GetCols(),
		       cvtype, ts);
    pImage->ReleaseBuffer();
   	if(!self->push_frame(x)) {
   		throw std::runtime_error("queue error");
   	}
}

void PointGrey::run() {
	this->connect();
	this->set_trigger_delay(0.0);
	this->init_configuration({});
	this->camera.StartCapture(on_image_grabbed, this);
	this->running = true;
}

void PointGrey::run(const Configuration& conf) {
	this->connect();
	this->set_trigger_delay(0.0);
	this->init_configuration(conf);
	this->camera.StartCapture(on_image_grabbed, this);
	this->running = true;
}

void PointGrey::init_configuration(const Configuration& conf) {
	if (this->first_run) {
		for (int i = 0; i < static_cast<int>(Property::num_property); ++i) {
			auto prop = conf.find(static_cast<Property>(i));
			CameraProperty c;
			c.type =  static_cast<Property>(i);
			
			if (prop != conf.end()) {
				// Found
				c.value = prop->second;
			} else {
				// Apply default property
				switch (c.type) {
				case Property::fps: {
					c.value = this->default_fps;
					break;
				}
				case Property::image_type: {
					c.value = this->default_image_type;
					break;
				}
				case Property::resolution: {
					c.value = this->default_resolution;
					break;
				}
				case Property::packet_delay: {
					c.value = this->default_packet_delay;	
					break;
				}
				case Property::packet_size: {
					c.value = this->default_packet_size;	
					break;
				}
				default:
					break;
				}
			}

			switch (c.type) {
			case Property::fps: {
				double fps = boost::any_cast<double>(c.value);
				this->set_fps(float(fps));
				break;
			}
			case Property::image_type: {
				this->set_image_type(boost::any_cast<ImageType>(c.value));
				break;
			}
			case Property::resolution: {
				this->set_resolution(boost::any_cast<Resolution>(c.value));
				break;
			}
			case Property::packet_delay: {
				int delay = boost::any_cast<int>(c.value);
				unsigned int delay_u = (unsigned int)delay;
				this->set_packet_delay(delay_u);	
				break;
			}
			case Property::packet_size: {
				int size = boost::any_cast<int>(c.value);
				unsigned int size_u = (unsigned int)size;
				this->set_packet_size(size_u);	
				break;
			}
			default:
				break;
			}
		
		}
		this->first_run = false;
	}
}

void PointGrey::stop() {
	this->camera.StopCapture();
	this->running = false;
	this->disconnect();
}

void PointGrey::wait_frame() {
	this->frames.wait();
}

bool PointGrey::get_frame(Image& image) {
	bool ret_value = this->frames.pop(image);
	if (ret_value == false) {
		return false;
	}

	while (ret_value) {
		ret_value = this->frames.pop(image);
	}
	return true;
}

bool PointGrey::get_frame(cv::Mat& img, int64_t& ts) {
	Image image;
	bool ret_value = this->frames.pop(image);
	if (ret_value == false) {
		return false;
	}

	while (ret_value) {
		ret_value = this->frames.pop(image);
	}
	
	img = cv::Mat(image.rows, image.cols, image.type);
	std::copy(image.frame.begin(), image.frame.end(), img.ptr());
	ts = image.timestamp;
	return true;
}

bool PointGrey::get_frame(cv::Mat& img) {
	Image image;
	bool ret_value = this->frames.pop(image);
	if (ret_value == false) {
		return false;
	}

	while (ret_value) {
		ret_value = this->frames.pop(image);
	}
	
	img = cv::Mat(image.rows, image.cols, image.type);
	std::copy(image.frame.begin(), image.frame.end(), img.ptr());
	return true;
}

bool PointGrey::push_frame(const Image& img) {
	return this->frames.push(img);
}

bool PointGrey::set_fps(float fps) {
	if (this->camera.IsConnected()) {
		Error error;
		FlyCapture2::Property frmRate;
		frmRate.type = FRAME_RATE;
		error = this->camera.GetProperty(&frmRate);
		if (error != PGRERROR_OK) {
			error.PrintErrorTrace();
			return false;
		}
		//validate FPS
		if (fps > this->get_max_fps()) {
			return false;
		}
		
		frmRate.autoManualMode = false;
		frmRate.absValue = fps;
		error = this->camera.SetProperty(&frmRate);
		if (error != PGRERROR_OK) {
			error.PrintErrorTrace();
			return false;
		}
		this->current_fps = fps;
		//this->set_trigger_delay(0.0);
		this->query_max_trigger_delay();
		return true;
	} else {
		return false;
	}
}

float PointGrey::get_fps() {
	return this->current_fps;
}

bool PointGrey::query_max_fps() {
	bool isconnected = this->camera.IsConnected();
	if(!isconnected) {
		this->connect();
	}
	Error error;
	PropertyInfo frmRateInfo;
	frmRateInfo.type = FRAME_RATE;
	error = this->camera.GetPropertyInfo(&frmRateInfo);
	if (error != PGRERROR_OK) {
		error.PrintErrorTrace();
		if(!isconnected) {
			this->disconnect();
		}
		return false;
	}
	this->max_fps = frmRateInfo.absMax;
	if (!isconnected) {
		this->disconnect();	
	}
	return true;
}

float PointGrey::get_max_fps() {
	return this->max_fps;
}
//TODO: verificar se da pra usar a função de triggerDelay da biblioteca
bool PointGrey::set_trigger_delay(float delay) {
	if (this->camera.IsConnected()) {
		Error error;
		FlyCapture2::Property triggerDelay;
		triggerDelay.type = TRIGGER_DELAY;
		error = this->camera.GetProperty(&triggerDelay);
		if (error != PGRERROR_OK) {
			error.PrintErrorTrace();
			return false;
		}
		//validate TriggerDelay
		if (delay/this->get_max_trigger_delay() > 1.01) {
			return false;
		}
		
		triggerDelay.onOff = true;
		triggerDelay.absValue = delay;
		error = this->camera.SetProperty(&triggerDelay);
		if (error != PGRERROR_OK) {
			error.PrintErrorTrace();
			return false;
		}
		return true;
	} else {
		return false;
	}
}

bool PointGrey::query_max_trigger_delay() {
	bool isconnected = this->camera.IsConnected();
	if(!isconnected) {
		this->connect();
	}
	Error error;
	PropertyInfo triggerDelayInfo;
	triggerDelayInfo.type = TRIGGER_DELAY;
	error = this->camera.GetPropertyInfo(&triggerDelayInfo);
	if (error != PGRERROR_OK) {
		error.PrintErrorTrace();
		if(!isconnected) {
			this->disconnect();
		}
		return false;
	}
	this->max_trigger_delay = triggerDelayInfo.absMax;
	if(!isconnected) {
		this->disconnect();
	}
	return true;
}

float PointGrey::get_max_trigger_delay() {
	return this->max_trigger_delay;
}

bool PointGrey::set_resolution(const Resolution& res) {
	int x, y;
	int width = res.width;
	int height = res.height;
	this->center_roi(x, y, width, height);
	return this->set_roi(x, y, width, height);
}

bool PointGrey::validate_roi(int& x, int& y, int& width, int& height) {
	// Adjust parameters
	// x
	while (x % this->hoffset != 0) {
		x++;
	}
	// y
	while (y % this->voffset != 0) {
		y++;
	}
	// width
	if (width % this->hstepsize != 0) {
		if ((width - 1) % this->hstepsize == 0) {
			width--;
		} else {
			while(width % this->hstepsize != 0) {
				width++;
			}
		}
	}
	width = width < (int)this->hstepsize ? (int)this->hstepsize : width;
	width = width > this->max_width ? this->max_width : width;
	// height
	while (height % this->voffset != 0) {
		height++;
	}
	height = height < (int)this->vstepsize ? (int)this->vstepsize : height;
	height = height > this->max_height ? this->max_height : height;
	// Validate ROI
	return ( (x + width <= this->max_width) &&
		     (y + height <= this->max_height) &&
		     ((x + width <= this->max_width) && (width != 0)) &&
		     ((y + height <= this->max_height) && (height != 0)) );
}

void PointGrey::center_roi(int& x, int& y, int& width, int& height) {
	this->validate_roi(x, y, width, height);

	x = (this->max_width - width) / 2;
	while (x % this->hoffset != 0) {
		x++;
	}

	y = (this->max_height - height) / 2;
	while (y % this->voffset != 0) {
		y++;
	}
}
/* 'x' means 'left', 'y' means 'top' */
bool PointGrey::set_roi(int& x, int& y, int& width, int& height) {
	// Validate ROI
	if ( this->validate_roi(x, y, width, height) ){
		// valid ROI
		bool rerun = this->running;
		bool isconnected;
		if (rerun) {
			this->stop();
			this->connect();
		} else {
			isconnected = this->camera.IsConnected();
			if(!isconnected) {
				this->connect();
			}
		}

		GigEImageSettings imageSettings;
		Error error;
		error = this->camera.GetGigEImageSettings(&imageSettings);
		if (error != PGRERROR_OK) {
			error.PrintErrorTrace();
			if (rerun) {
				this->run();
				return false;
			}
			if(!isconnected) {
				this->disconnect();
				return false;
			}
		}
		if ( (imageSettings.offsetX == (unsigned int)x) &&
			 (imageSettings.offsetY == (unsigned int)y) &&
			 (imageSettings.width   == (unsigned int)width) && 
			 (imageSettings.height  == (unsigned int)height) ) {
			// no changes on ROI
			if (rerun) {
				this->run();
				return true;
			}
			if(!isconnected) {
				this->disconnect();
				return true;
			}
		}
		
		imageSettings.offsetX = (unsigned int)x;
		imageSettings.offsetY = (unsigned int)y;
		imageSettings.width   = (unsigned int)width;
		imageSettings.height  = (unsigned int)height;

		error = this->camera.SetGigEImageSettings(&imageSettings);
		if (error != PGRERROR_OK) {
			error.PrintErrorTrace();
			if (rerun) {
				this->run();
				return false;
			}
			if(!isconnected) {
				this->disconnect();
				return false;
			}
		}
		// No error. FPS may change when resolution changes
		this->query_max_fps();
		if (rerun) {
			this->run();
			return true;
		}
		if(!isconnected) {
			this->disconnect();
			return true;
		}
	} else {
		return false;
	}
}

bool PointGrey::query_max_resolution() {
	bool isconnected = this->camera.IsConnected();
	if(!isconnected) {
		this->connect();
	}
	Error error;
	GigEImageSettingsInfo imageInfo;
	error = this->camera.GetGigEImageSettingsInfo(&imageInfo);
	if (error != PGRERROR_OK) {
		error.PrintErrorTrace();
		if(!isconnected) {
			this->disconnect();
		}
		return false;
	}
	this->max_width = imageInfo.maxWidth;
	this->max_height= imageInfo.maxHeight;
	this->hoffset = imageInfo.offsetHStepSize;
	this->voffset = imageInfo.offsetVStepSize;
	this->hstepsize = imageInfo.imageHStepSize;
	this->vstepsize = imageInfo.imageVStepSize;

	if(!isconnected) {
		this->disconnect();
	}
	return true;
}

bool PointGrey::set_packet_delay(unsigned int delay) {
	if (!this->running) {
		bool isconnected = this->camera.IsConnected();
		if(!isconnected) {
			this->connect();
		}
		Error error;
		GigEProperty p;
		
		p.propType = FlyCapture2::PACKET_DELAY;
		error = this->camera.GetGigEProperty(&p);
		if (error != PGRERROR_OK) {
			error.PrintErrorTrace();
			if(!isconnected) {
				this->disconnect();
			}
			return false;
		}
		delay = delay > p.max ? p.max : delay;
		delay = delay < p.min ? p.min : delay;
		p.value = delay;
		error = this->camera.SetGigEProperty(&p);
		if (error != PGRERROR_OK) {
			error.PrintErrorTrace();
			if(!isconnected) {
				this->disconnect();
			}
			return false;
		}
		if(!isconnected) {
			this->disconnect();
		}
		return true;
	} else {
		return false;
	}
}

bool PointGrey::set_packet_size(unsigned int size) {
	if (!this->running) {
		bool isconnected = this->camera.IsConnected();
		if(!isconnected) {
			this->connect();
		}
		Error error;
		GigEProperty p;

		p.propType = FlyCapture2::PACKET_SIZE;
		error = this->camera.GetGigEProperty(&p);
		if (error != PGRERROR_OK) {
			error.PrintErrorTrace();
			if(!isconnected) {
				this->disconnect();
			}			
			return false;
		}
		size = size > p.max ? p.max : size;
		size = size < p.min ? p.min : size;
		p.value = size;
		error = this->camera.SetGigEProperty(&p);
		if (error != PGRERROR_OK) {
			error.PrintErrorTrace();
			if(!isconnected) {
				this->disconnect();
			}
			return false;
		}
		if(!isconnected) {
			this->disconnect();
		}
		return true;
	} else {
		return false;
	}
}

ImageType PointGrey::get_image_type() {
	return this->imtype.load();
}

void PointGrey::set_image_type(const ImageType& type) {
	this->imtype.store(type);
}

void PointGrey::set_property(CameraProperty& prop) {
	switch (prop.type) {
	case Property::fps:
		this->set_fps(boost::any_cast<float>(prop.value));
		break;
	case Property::image_type:
		this->set_image_type(boost::any_cast<ImageType>(prop.value));
		break;
	case Property::resolution:
		this->set_resolution(boost::any_cast<Resolution>(prop.value));
		break;
	case Property::delay:
		this->set_trigger_delay(boost::any_cast<float>(prop.value));
		break;
	default:
		break;
	}
}

bool PointGrey::query_serial_number() {
	bool isconnected = this->camera.IsConnected();
	if(!isconnected) {
		this->connect();
	}
	Error error;
    CameraInfo camInfo;
    error = this->camera.GetCameraInfo(&camInfo);
	if (error != PGRERROR_OK) {
		error.PrintErrorTrace();
		if(!isconnected) {
			this->disconnect();
		}
		return false;
	}
	this->serial_number = camInfo.serialNumber;
	if(!isconnected) {
		this->disconnect();
	}
	return true;
}

unsigned int PointGrey::get_serial_number() {
	return this->serial_number;
}

void PointGrey::print_camera_info() {
    CameraInfo camInfo;
	this->camera.Connect(this->handle);
    this->camera.GetCameraInfo(&camInfo);
	this->camera.Disconnect();

	ostringstream info;
	info << "[" << camInfo.vendorName << "]" <<
			"[" << camInfo.modelName << "]" <<
			"[" << camInfo.sensorResolution << "]" <<
			"[" << (unsigned int)camInfo.ipAddress.octets[0] << "." <<
				   (unsigned int)camInfo.ipAddress.octets[1] << "." <<
				   (unsigned int)camInfo.ipAddress.octets[2] << "." << 
				   (unsigned int)camInfo.ipAddress.octets[3] << "]" <<
			"[" << camInfo.serialNumber << "]\n";
	cout << info.str();
}

bool PointGrey::sort_by_serial(vector<unique_ptr<PointGrey>>& cameras,
				 			   const vector<unsigned int>& serials) {
	if (cameras.size() != serials.size()) {
		return false;
	}
	for (int i=0; i<serials.size(); ++i){
		for (int j=0; j<cameras.size(); ++j) {
			if (serials[i] == cameras[j]->get_serial_number()) {
				std::swap(cameras[j],cameras[i]);
				break;
			}
		}
	}
	return true;
}

} // :: cameras
} // ::is
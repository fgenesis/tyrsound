/*
 * libopenmpt_impl.hpp
 * -------------------
 * Purpose: libopenmpt private interface
 * Notes  : This is not a public header. Do NOT ship in distributions dev packages.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#ifndef LIBOPENMPT_IMPL_HPP
#define LIBOPENMPT_IMPL_HPP

#include "libopenmpt_internal.h"
#include "libopenmpt.hpp"


#include <memory>
#include <ostream>

// forward declarations
namespace OpenMPT {
class FileReader;
class CSoundFile;
class Dither;
} // namespace OpenMPT

namespace openmpt {

namespace version {

uint32 get_library_version();
uint32 get_core_version();
std::string get_string( const std::string & key );

} // namespace version

class log_interface {
protected:
	log_interface();
public:
	virtual ~log_interface();
	virtual void log( const std::string & message ) const = 0;
}; // class log_interface

class std_ostream_log : public log_interface {
private:
	std::ostream & destination;
public:
	std_ostream_log( std::ostream & dst );
	virtual ~std_ostream_log();
	virtual void log( const std::string & message ) const;
}; // class CSoundFileLog_std_ostream


class log_forwarder;

class module_impl {
protected:
	log_interface *m_Log;
	log_forwarder *m_LogForwarder;
	double m_currentPositionSeconds;
	OpenMPT::CSoundFile *m_sndFile;
	OpenMPT::Dither  *m_Dither;
	float m_Gain;
	bool m_ctl_load_skip_samples;
	bool m_ctl_load_skip_patterns;
	std::vector<std::string> m_loaderMessages;
public:
	void PushToCSoundFileLog( const std::string & text ) const;
	void PushToCSoundFileLog( int loglevel, const std::string & text ) const;
protected:
	std::string mod_string_to_utf8( const std::string & encoded ) const;
	void apply_mixer_settings( int32 samplerate, int channels );
	void apply_libopenmpt_defaults();
	void init( const std::map< std::string, std::string > & ctls );
	void load( OpenMPT::CSoundFile & sndFile, const OpenMPT::FileReader & file );
	void load( const OpenMPT::FileReader & file );
	std::size_t read_wrapper( std::size_t count, int16 * left, int16 * right, int16 * rear_left, int16 * rear_right );
	std::size_t read_wrapper( std::size_t count, float * left, float * right, float * rear_left, float * rear_right );
	std::size_t read_interleaved_wrapper( std::size_t count, std::size_t channels, int16 * interleaved );
	std::size_t read_interleaved_wrapper( std::size_t count, std::size_t channels, float * interleaved );
	std::pair< std::string, std::string > format_and_highlight_pattern_row_channel_command( int32 p, int32 r, int32 c, int command ) const;
	std::pair< std::string, std::string > format_and_highlight_pattern_row_channel( int32 p, int32 r, int32 c, std::size_t width, bool pad ) const;
public:
	static std::vector<std::string> get_supported_extensions();
	static bool is_extension_supported( const std::string & extension );
	static double could_open_propability( std::istream & stream, double effort, log_interface *log );
	module_impl( std::istream & stream, log_interface * log, const std::map< std::string, std::string > & ctls );
	module_impl( const std::vector<std::uint8_t> & data, log_interface * log, const std::map< std::string, std::string > & ctls );
	module_impl( const std::vector<char> & data, log_interface * log, const std::map< std::string, std::string > & ctls );
	module_impl( const std::uint8_t * data, std::size_t size, log_interface * log, const std::map< std::string, std::string > & ctls );
	module_impl( const char * data, std::size_t size, log_interface * log, const std::map< std::string, std::string > & ctls );
	module_impl( const void * data, std::size_t size, log_interface *log, const std::map< std::string, std::string > & ctls );
	~module_impl();
public:
	void select_subsong( int32 subsong );
	void set_repeat_count( int32 repeat_count );
	int32 get_repeat_count() const;
	double get_duration_seconds() const;
	double set_position_seconds( double seconds );
	double get_position_seconds() const;
	double set_position_order_row( int32 order, int32 row );
	int32 get_render_param( int param ) const;
	void set_render_param( int param, int32 value );
	std::size_t read( int32 samplerate, std::size_t count, int16 * mono );
	std::size_t read( int32 samplerate, std::size_t count, int16 * left, int16 * right );
	std::size_t read( int32 samplerate, std::size_t count, int16 * left, int16 * right, int16 * rear_left, int16 * rear_right );
	std::size_t read( int32 samplerate, std::size_t count, float * mono );
	std::size_t read( int32 samplerate, std::size_t count, float * left, float * right );
	std::size_t read( int32 samplerate, std::size_t count, float * left, float * right, float * rear_left, float * rear_right );
	std::size_t read_interleaved_stereo( int32 samplerate, std::size_t count, int16 * interleaved_stereo );
	std::size_t read_interleaved_quad( int32 samplerate, std::size_t count, int16 * interleaved_quad );
	std::size_t read_interleaved_stereo( int32 samplerate, std::size_t count, float * interleaved_stereo );
	std::size_t read_interleaved_quad( int32 samplerate, std::size_t count, float * interleaved_quad );
	std::vector<std::string> get_metadata_keys() const;
	std::string get_metadata( const std::string & key ) const;
	int32 get_current_speed() const;
	int32 get_current_tempo() const;
	int32 get_current_order() const;
	int32 get_current_pattern() const;
	int32 get_current_row() const;
	int32 get_current_playing_channels() const;
	float get_current_channel_vu_mono( int32 channel ) const;
	float get_current_channel_vu_left( int32 channel ) const;
	float get_current_channel_vu_right( int32 channel ) const;
	float get_current_channel_vu_rear_left( int32 channel ) const;
	float get_current_channel_vu_rear_right( int32 channel ) const;
	int32 get_num_subsongs() const;
	int32 get_num_channels() const;
	int32 get_num_orders() const;
	int32 get_num_patterns() const;
	int32 get_num_instruments() const;
	int32 get_num_samples() const;
	std::vector<std::string> get_subsong_names() const;
	std::vector<std::string> get_channel_names() const;
	std::vector<std::string> get_order_names() const;
	std::vector<std::string> get_pattern_names() const;
	std::vector<std::string> get_instrument_names() const;
	std::vector<std::string> get_sample_names() const;
	int32 get_order_pattern( int32 o ) const;
	int32 get_pattern_num_rows( int32 p ) const;
	uint8 get_pattern_row_channel_command( int32 p, int32 r, int32 c, int cmd ) const;
	std::string format_pattern_row_channel_command( int32 p, int32 r, int32 c, int cmd ) const;
	std::string highlight_pattern_row_channel_command( int32 p, int32 r, int32 c, int cmd ) const;
	std::string format_pattern_row_channel( int32 p, int32 r, int32 c, std::size_t width, bool pad ) const;
	std::string highlight_pattern_row_channel( int32 p, int32 r, int32 c, std::size_t width, bool pad ) const;
	std::vector<std::string> get_ctls() const;
	std::string ctl_get( const std::string & ctl ) const;
	void ctl_set( const std::string & ctl, const std::string & value );
}; // class module_impl

} // namespace openmpt

#endif // LIBOPENMPT_IMPL_HPP

/* libvorbisfile_winrt - Ogg Vorbis for Windows Runtime
 * Copyright (C) 2014  Alexander Ovchinnikov
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * - Neither the name of copyright holder nor the names of project's
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "vorbis/vorbis_winrt.h"
#include <inspectable.h>
#include <robuffer.h>
#include <ppltasks.h>

/* get internal IBuffer array */
static
uint8 *get_array(Windows::Storage::Streams::IBuffer^ buffer)
{
	IInspectable *inspectable = reinterpret_cast<IInspectable *>(buffer);
	Microsoft::WRL::ComPtr<Windows::Storage::Streams::IBufferByteAccess> spBuffAccess;
	HRESULT hr = inspectable->QueryInterface(__uuidof(Windows::Storage::Streams::IBufferByteAccess), (void **)&spBuffAccess);
	uint8 *pReadBuffer = nullptr;
	spBuffAccess->Buffer(&pReadBuffer);
	return pReadBuffer;
}

namespace libvorbisfile {

	namespace WindowsRuntime {

		OggVorbisFile::OggVorbisFile()
			: file_stream_(nullptr)
		{ }

		OggVorbisFile::~OggVorbisFile()
		{
			Clear();
		}

		bool OggVorbisFile::IsValid::get()
		{
			return nullptr != vf_ && nullptr != file_stream_ && nullptr != file_reader_;
		}

		void OggVorbisFile::Open(Windows::Storage::Streams::IRandomAccessStream^ fileStream)
		{
			Open(fileStream, nullptr);
		}

		void OggVorbisFile::Open(Windows::Storage::Streams::IRandomAccessStream^ fileStream, Windows::Storage::Streams::IBuffer^ initial)
		{
			Clear();
			vf_ = new ::OggVorbis_File();

			file_stream_ = fileStream;
			file_reader_ = ref new Windows::Storage::Streams::DataReader(file_stream_);

			::ov_callbacks callbacks = {
				read_func_,
				seek_func_,
				close_func_,
				tell_func_
			};

			int ret;
			if (nullptr != initial) {
				char *initial_array = reinterpret_cast<char *>(get_array(initial));
				ret = ::ov_open_callbacks((void *)this, vf_, const_cast<const char *>(initial_array), (long)initial->Length, callbacks);
			}
			else {
				ret = ::ov_open_callbacks((void *)this, vf_, nullptr, 0, callbacks);
			}

			if (ret < 0) {
				Clear();
				throw Platform::Exception::CreateException(ret);
			}
		}

		void OggVorbisFile::Clear()
		{
			if (vf_) {
				(void)::ov_clear(vf_);
				delete vf_;
				vf_ = nullptr;
			}

			if (nullptr != file_reader_)
			{
				(void)file_reader_->DetachStream();
				delete file_reader_;
			}
			file_reader_ = nullptr;
			file_stream_ = nullptr;
		}

		OggVorbisFile^ OggVorbisFile::TestOpen(Windows::Storage::Streams::IRandomAccessStream^ fileStream)
		{
			return OggVorbisFile::TestOpen(fileStream, nullptr);
		}

		OggVorbisFile^ OggVorbisFile::TestOpen(Windows::Storage::Streams::IRandomAccessStream^ fileStream, Windows::Storage::Streams::IBuffer^ initial)
		{
			OggVorbisFile^ file = ref new OggVorbisFile();
			file->vf_ = new OggVorbis_File();

			file->file_stream_ = fileStream;
			file->file_reader_ = ref new Windows::Storage::Streams::DataReader(file->file_stream_);

			::ov_callbacks callbacks = {
				read_func_,
				seek_func_,
				close_func_,
				tell_func_
			};

			int ret;

			if (nullptr != initial) {
				char *initial_array = reinterpret_cast<char *>(get_array(initial));
				ret = ::ov_test_callbacks((void *)file, file->vf_, const_cast<const char *>(initial_array), (long)initial->Length, callbacks);
			}
			else {
				ret = ::ov_test_callbacks((void *)file, file->vf_, nullptr, 0, callbacks);
			}

			if (ret < 0) {
				file->Clear();
				throw Platform::Exception::CreateException(ret);
			}

			ret = ::ov_test_open(file->vf_);
			if (ret < 0) {
				file->Clear();
				throw Platform::Exception::CreateException(ret);
			}

			return file;
		}

		Windows::Storage::Streams::IBuffer^ OggVorbisFile::Read(int length)
		{
			int dummy;
			return Read(length, &dummy);
		}

		Windows::Storage::Streams::IBuffer^ OggVorbisFile::Read(int length, int *bitstream)
		{
			assert(IsValid);

			Windows::Storage::Streams::IBuffer^ buffer = ref new Windows::Storage::Streams::Buffer((unsigned)length);
			uint8 *buffer_array = get_array(buffer);

			long ret = ::ov_read(vf_, reinterpret_cast<char *>(buffer_array), length, 0, 2, 1, bitstream);
			if (ret < 0)
				throw Platform::Exception::CreateException(ret);

			buffer->Length = ret;
			return buffer;
		}

		void OggVorbisFile::Crosslap(OggVorbisFile^ oldFile, OggVorbisFile^ newFile)
		{
			assert(oldFile->IsValid);
			assert(newFile->IsValid);
			int ret = ::ov_crosslap(oldFile->vf_, newFile->vf_);
			if (ret < 0)
				throw Platform::Exception::CreateException(ret);
		}

		void OggVorbisFile::RawSeek(ogg_int64_t pos)
		{
			assert(IsValid);
			int ret = ::ov_raw_seek(vf_, pos);
			if (OV_EINVAL == ret)
				throw ref new Platform::InvalidArgumentException();
			if (OV_EFAULT == ret)
				throw ref new Platform::FailureException();
			if (ret < 0)
				throw Platform::Exception::CreateException(ret);
		}

		void OggVorbisFile::PcmSeek(ogg_int64_t pos)
		{
			assert(IsValid);
			int ret = ::ov_pcm_seek(vf_, pos);
			if (OV_EINVAL == ret)
				throw ref new Platform::InvalidArgumentException();
			if (OV_EFAULT == ret)
				throw ref new Platform::FailureException();
			if (ret < 0)
				throw Platform::Exception::CreateException(ret);
		}

		void OggVorbisFile::TimeSeek(double pos)
		{
			assert(IsValid);
			int ret = ::ov_time_seek(vf_, pos);
			if (OV_EINVAL == ret)
				throw ref new Platform::InvalidArgumentException();
			if (OV_EFAULT == ret)
				throw ref new Platform::FailureException();
			if (ret < 0)
				throw Platform::Exception::CreateException(ret);
		}

		void OggVorbisFile::PcmSeekPage(ogg_int64_t pos)
		{
			assert(IsValid);
			int ret = ::ov_pcm_seek_page(vf_, pos);
			if (OV_EINVAL == ret)
				throw ref new Platform::InvalidArgumentException();
			if (OV_EFAULT == ret)
				throw ref new Platform::FailureException();
			if (ret < 0)
				throw Platform::Exception::CreateException(ret);
		}

		void OggVorbisFile::TimeSeekPage(double pos)
		{
			assert(IsValid);
			int ret = ::ov_time_seek_page(vf_, pos);
			if (OV_EINVAL == ret)
				throw ref new Platform::InvalidArgumentException();
			if (OV_EFAULT == ret)
				throw ref new Platform::FailureException();
			if (ret < 0)
				throw Platform::Exception::CreateException(ret);
		}

		void OggVorbisFile::RawSeekLap(ogg_int64_t pos)
		{
			assert(IsValid);
			int ret = ::ov_raw_seek_lap(vf_, pos);
			if (OV_EINVAL == ret)
				throw ref new Platform::InvalidArgumentException();
			if (OV_EFAULT == ret)
				throw ref new Platform::FailureException();
			if (ret < 0)
				throw Platform::Exception::CreateException(ret);
		}

		void OggVorbisFile::PcmSeekLap(ogg_int64_t pos)
		{
			assert(IsValid);
			int ret = ::ov_pcm_seek_lap(vf_, pos);
			if (OV_EINVAL == ret)
				throw ref new Platform::InvalidArgumentException();
			if (OV_EFAULT == ret)
				throw ref new Platform::FailureException();
			if (ret < 0)
				throw Platform::Exception::CreateException(ret);
		}

		void OggVorbisFile::TimeSeekLap(double pos)
		{
			assert(IsValid);
			int ret = ::ov_time_seek_lap(vf_, pos);
			if (OV_EINVAL == ret)
				throw ref new Platform::InvalidArgumentException();
			if (OV_EFAULT == ret)
				throw ref new Platform::FailureException();
			if (ret < 0)
				throw Platform::Exception::CreateException(ret);
		}

		void OggVorbisFile::PcmSeekPageLap(ogg_int64_t pos)
		{
			assert(IsValid);
			int ret = ::ov_pcm_seek_page_lap(vf_, pos);
			if (OV_EINVAL == ret)
				throw ref new Platform::InvalidArgumentException();
			if (OV_EFAULT == ret)
				throw ref new Platform::FailureException();
			if (ret < 0)
				throw Platform::Exception::CreateException(ret);
		}

		void OggVorbisFile::TimeSeekPageLap(double pos)
		{
			assert(IsValid);
			int ret = ::ov_time_seek_page_lap(vf_, pos);
			if (OV_EINVAL == ret)
				throw ref new Platform::InvalidArgumentException();
			if (OV_EFAULT == ret)
				throw ref new Platform::FailureException();
			if (ret < 0)
				throw Platform::Exception::CreateException(ret);
		}

		int OggVorbisFile::Bitrate()
		{
			return Bitrate(-1);
		}

		int OggVorbisFile::Bitrate(int bitstream)
		{
			assert(IsValid);
			int ret = ::ov_bitrate(vf_, bitstream);
			if (OV_EINVAL == ret)
				throw ref new Platform::InvalidArgumentException();
			if (ret < 0)
				throw Platform::Exception::CreateException(ret);
			return ret;
		}

		int OggVorbisFile::BitrateInstant()
		{
			assert(IsValid);
			int ret = ::ov_bitrate_instant(vf_);
			if (OV_EINVAL == ret)
				throw ref new Platform::InvalidArgumentException();
			if (ret < 0)
				throw Platform::Exception::CreateException(ret);
			return ret;
		}

		int OggVorbisFile::Streams()
		{
			assert(IsValid);
			return ::ov_streams(vf_);
		}

		bool OggVorbisFile::Seekable()
		{
			assert(IsValid);
			return !!(::ov_seekable(vf_));
		}

		int OggVorbisFile::SerialNumber()
		{
			return SerialNumber(-1);
		}

		int OggVorbisFile::SerialNumber(int bitstream)
		{
			assert(IsValid);
			int ret = ::ov_serialnumber(vf_, bitstream);
			if (-1 == ret)
				throw ref new Platform::InvalidArgumentException("bitstream does not exist");
			return ret;
		}

		ogg_int64_t OggVorbisFile::RawTotal()
		{
			return RawTotal(-1);
		}

		ogg_int64_t OggVorbisFile::RawTotal(int bitstream)
		{
			assert(IsValid);
			ogg_int64_t ret = ::ov_raw_total(vf_, bitstream);
			if (OV_EINVAL == ret)
				throw ref new Platform::InvalidArgumentException();
			return ret;
		}

		ogg_int64_t OggVorbisFile::PcmTotal()
		{
			return PcmTotal(-1);
		}

		ogg_int64_t OggVorbisFile::PcmTotal(int bitstream)
		{
			assert(IsValid);
			ogg_int64_t ret = ::ov_pcm_total(vf_, bitstream);
			if (OV_EINVAL == ret)
				throw ref new Platform::InvalidArgumentException();
			return ret;
		}

		double OggVorbisFile::TimeTotal()
		{
			return TimeTotal(-1);
		}

		double OggVorbisFile::TimeTotal(int bitstream)
		{
			assert(IsValid);
			double ret = ::ov_time_total(vf_, bitstream);
			if (OV_EINVAL == (int)ret)
				throw ref new Platform::InvalidArgumentException();
			return ret;
		}

		ogg_int64_t OggVorbisFile::RawTell()
		{
			assert(IsValid);
			ogg_int64_t ret = ::ov_raw_tell(vf_);
			if (OV_EINVAL == ret)
				throw ref new Platform::InvalidArgumentException();
			return ret;
		}

		ogg_int64_t OggVorbisFile::PcmTell()
		{
			assert(IsValid);
			ogg_int64_t ret = ::ov_pcm_tell(vf_);
			if (OV_EINVAL == ret)
				throw ref new Platform::InvalidArgumentException();
			return ret;
		}

		double OggVorbisFile::TimeTell()
		{
			assert(IsValid);
			double ret = ::ov_time_tell(vf_);
			if (OV_EINVAL == (int)ret)
				throw ref new Platform::InvalidArgumentException();
			return ret;
		}

		VorbisInfo^ OggVorbisFile::Info()
		{
			return Info(-1);
		}

		VorbisInfo^ OggVorbisFile::Info(int bitstream)
		{
			assert(IsValid);
			::vorbis_info *vi = ::ov_info(vf_, bitstream);
			return vi ? ref new VorbisInfo(vi) : nullptr;
		}

		VorbisComment^ OggVorbisFile::Comment()
		{
			return Comment(-1);
		}

		VorbisComment^ OggVorbisFile::Comment(int bitstream)
		{
			assert(IsValid);
			::vorbis_comment *vc = ::ov_comment(vf_, bitstream);
			return vc ? ref new VorbisComment(vc) : nullptr;
		}


		size_t OggVorbisFile::read_func_(void *ptr, size_t size, size_t nmemb, void *datasource)
		{
			OggVorbisFile^ instance = reinterpret_cast<OggVorbisFile^>(datasource);
			assert(nullptr != instance && instance->IsValid);
			if (nmemb > 0) {
				unsigned count = concurrency::create_task(instance->file_reader_->LoadAsync(size * nmemb)).get();
				if (count > 0) {
					instance->file_reader_->ReadBytes(Platform::ArrayReference<uint8>(reinterpret_cast<uint8 *>(ptr), count));
					return count;
				}
			}
			return 0;
		}

		int OggVorbisFile::seek_func_(void *datasource, ogg_int64_t offset, int whence)
		{
			OggVorbisFile^ instance = reinterpret_cast<OggVorbisFile^>(datasource);
			assert(nullptr != instance && instance->IsValid);
			switch (whence) {
			case SEEK_CUR:
				instance->file_stream_->Seek(instance->file_stream_->Position + (uint64)offset);
				break;
			case SEEK_END:
				instance->file_stream_->Seek(instance->file_stream_->Size - (uint64)offset);
				break;
			case SEEK_SET:
				instance->file_stream_->Seek((uint64)offset);
				break;
			default:
				throw ref new Platform::InvalidArgumentException("whence");
			}
			return 0;
		}

		int OggVorbisFile::close_func_(void *datasource)
		{
			OggVorbisFile^ instance = reinterpret_cast<OggVorbisFile^>(datasource);
			assert(nullptr != instance && instance->IsValid);
			(void)instance->file_reader_->DetachStream();
			delete instance->file_reader_;
			instance->file_reader_ = nullptr;
			return 0;
		}

		long OggVorbisFile::tell_func_(void *datasource)
		{
			OggVorbisFile^ instance = reinterpret_cast<OggVorbisFile^>(datasource);
			assert(nullptr != instance && instance->IsValid);
			return (long)instance->file_stream_->Position;
		}

	}

}

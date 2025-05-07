/* Streaming video compression high level functions */
/* Author: B.A. Sobhani <sobhaniba@ornl.gov> */

#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include "codec_tools.h"
extern int mutex_initialized;
int a=0;
int compress_buffer(CodecContext** pc, char* source, int x_size, int y_size, char* dest){
	//AVCodecContext* c_c = c->c_c;
	printf("vc pointer in compres_buffer %d\n", pc);
	printf("compress buffer codeccontext %d\n", *pc);
	printf("compress buffer int codeccontext %d\n", *((int*)pc));
	printf("compress buffer char codeccontext %d\n", *((char*)pc));
	
	int pix_size = 1;
	int uncompressed_size = pix_size*x_size*y_size;
	printf("x_size %d y_size %d\n", x_size, y_size);
	//if(a==0) printf("Warning a=0\n");
	CodecContext* c;
	if(*pc == 0 || a==0){
		printf("here1\n");
		a=1;
		*pc = init_encoder_context(x_size, y_size); 
		c = *pc;
		//c_c = init_av_encoder_context(x_size, y_size); 
		//c->c_c = init_av_encoder_context(x_size, y_size); 
		printf("initialized width %d height %d\n", (*pc)->c_c->width, (*pc)->c_c->height);
		printf("here2\n");
		if(c->c_c==0){
			printf("Failed to initialize context\n");
			return 0;
		}
		printf("end of first if\n");
	}
	else if((*pc)->c_c->width!=x_size || (*pc)->c_c->height!=y_size){
		c = *pc;
		printf("else if\n");
		avcodec_close(c->c_c);
		//c_c = init_encoder_context(x_size, y_size); 
		c->c_c = init_av_encoder_context(x_size, y_size); 
	}
		printf("end of if block\n");
		c = *pc;
	pthread_mutex_t* mutex = &((*pc)->mutex);
	printf("mutex addr compress %d\n", mutex);
	//if(c->mutex_initialized==0){
	if(mutex_initialized==0){
		printf("initializing mutex\n");
		pthread_mutex_init(mutex, NULL);
		(*pc)->mutex_initialized=1;
		mutex_initialized=1;
	}
	pthread_mutex_lock(mutex);
		
	AVPacket* pkt;
	AVFrame* frame = uncompressed_buffer_to_frame(c->c_c, source);
	pkt = frame_to_packet(c->c_c, frame);
	free(frame->data[0]);
	av_frame_free(&frame);
	if(pkt==0){
		pthread_mutex_unlock(mutex);
		return -1;
	}
	//Fail if compressed size is larger than uncompressed - to prevent memory being written out of range
	if(pkt->size > uncompressed_size){
		pthread_mutex_unlock(mutex);
		return -1;
	}
	packet_to_buffer(pkt, dest);
	int r = pkt->size;
	av_packet_free(&pkt);
	//av_packet_unref(pkt);
	//return pkt->size;
	pthread_mutex_unlock(mutex);
	return r;
}
AVCodecContext* init_av_decoder_context();
int decompress_buffer(char* buffer_in, int size_in, char* buffer_out, int* width, int* height){
	if(c_d==0) c_d = init_av_decoder_context();
	AVPacket* pkt;
	pkt = av_packet_alloc();
	pkt->pts = count_d;
	buffer_to_packet(buffer_in, size_in, pkt);
	AVFrame* frame = packet_to_frame(c_d, pkt);
	int pt = frame->pict_type;
	if(first==1 && pt != AV_PICTURE_TYPE_I){
		av_frame_free(&frame);
		av_packet_free(&pkt);
		return -1;
	}
	first = 0;
	count_d++;
	int decompressed_size = frame_to_buffer(frame, buffer_out, width, height);
	//free(frame->data[0]);
	av_frame_free(&frame);
	av_packet_free(&pkt);
	return decompressed_size;
}


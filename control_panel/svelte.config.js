import cloudflareAdapter from '@sveltejs/adapter-cloudflare';
import staticAdapter from '@sveltejs/adapter-static'
import nodeAdapter from '@sveltejs/adapter-node'
import multiAdapter from '@macfja/svelte-multi-adapter';
import preprocess from 'svelte-preprocess';

/** @type {import('@sveltejs/kit').Config} */
const config = {
	preprocess: preprocess(),
	kit: {
		adapter: multiAdapter([
			cloudflareAdapter({
				target: 'es2020',
				platform: 'browser',
				bundle: true,
			}),
			nodeAdapter({
				out: 'build',
				precompress: true
			}),
			staticAdapter({
				pages: 'public_html',
				assets: 'public_html',
				precompress: true
			})
		]),
		target: '#svelte'
	}
};

export default config;

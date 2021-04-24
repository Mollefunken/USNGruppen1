const cacheName = 'v1';

const cacheAssets = 
[
    'jsindex.html',
    'fetch.js',
    'main.js',
    'jsstyle.css'
];

//Call install event
self.addEventListener('install', (event) => {
    console.log('Service Worker: Installed');

    event.waitUntil
    (
        caches
            .open(cacheName)
            .then(cache => 
                {
                    console.log('Service Worker: Caching Files');
                    cache.addAll(cacheAssets);
                })
                .then(() => self.skipWaiting())
    );
});

//Call activate event
self.addEventListener('activate', (event) => 
{
    console.log('Service Worker: Activated');
    //Remove unwanted caches
    (event).waitUntil
    (
        caches.keys().then(cacheNames => 
            {
                return Promise.all
                (
                    cacheNames.map(cache => 
                    {
                        if(cache !== cacheName) 
                            {
                                console.log('Service Worker: Clearing Old Cache');
                                return caches.delete(cache);
                            }
                    })
                )
            })
    );
});

//Call Fetch Event
/*self.addEventListener('fetch', (event) => 
{
    console.log('Service Worker: Fetching');
    (event).respondWith
    (
        fetch((event).request).catch(() => caches.match((event). request))
    );
})*/

//Call Fetch Event
self.addEventListener('fetch', (event) => 
{
   /* if (event.request.method == 'OPTIONS') {
	console.log('Service Worker: Options');
	(event).respondWith (async function() {
		headers: {
			'200 OK',
			'Access-Control-Allow-Origin: *',
			'Access-Control-Allow-Methods: GET, POST, PUT, DELETE' };
	});
    }*/
    if (event.request.method != 'GET') return;

    console.log('Service Worker: Fetching');
    (event).respondWith
    (
        fetch((event).request)
            .then(res => {
            // Make copy/clone of response
            const resClone = res.clone();
            //open cache
            caches
                .open(cacheName)
                .then(cache => 
                    {
                        //add response to cache
                        cache.put((event).request, resClone);
                    });
                return res;
        })
        .catch(err => caches.match((event).request).then(res => res))
    );
});
